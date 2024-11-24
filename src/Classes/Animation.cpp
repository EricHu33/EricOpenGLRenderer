#include "Animation.h"
#include <assimp/anim.h>
#include <glm/gtc/type_ptr.hpp>
#include "../assimp_glm_helpers.h"
#include "../Scene.h"


Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
        :
        m_name(name),
        m_ID(ID),
        m_localTransform(1.0f)
{
    m_numPositions = channel->mNumPositionKeys;
    m_numRotations = channel->mNumRotationKeys;
    m_numScales = channel->mNumScalingKeys;

    for (int index = 0; index < m_numPositions; ++index)
    {
        aiVector3D aiPosition = channel->mPositionKeys[index].mValue;
        float timeStamp = channel->mPositionKeys[index].mTime;
        KeyPosition data;
        data.position = glm::vec4(AssimpGLMHelpers::GetGLMVec(aiPosition), 0);
        data.timeStamp = timeStamp;
        m_positons.push_back(data);
    }

    for (int index = 0; index < m_numRotations; ++index)
    {
        aiQuaternion aiOrientation = channel->mRotationKeys[index].mValue;
        float timeStamp = channel->mRotationKeys[index].mTime;
        KeyRotation data;
        data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
        data.timeStamp = timeStamp;
        m_rotations.push_back(data);
    }

    for (int index = 0; index < m_numRotations; ++index)
    {
        aiVector3D scale = channel->mScalingKeys[index].mValue;
        float timeStamp = channel->mScalingKeys[index].mTime;
        KeyScale data;
        data.scale = AssimpGLMHelpers::GetGLMVec(scale);
        data.timeStamp = timeStamp;
        m_scales.push_back(data);
    }
}

int Bone::GetPositionIndex(float animationTime)
{
    for (int index = 0; index < m_numPositions-1; ++index)
    {
        if (index == m_numPositions - 1 || animationTime < m_positons[index + 1].timeStamp)
            return index;
    }
   
    assert(0);
}

int Bone::GetRotationIndex(float animationTime)
{
    for (int index = 0; index < m_numRotations-1; ++index)
    {
        if (index == m_numRotations - 1 || animationTime < m_rotations[index + 1].timeStamp)
            return index;
    }
    assert(0);
}

int Bone::GetScaleIndex(float animationTime)
{
    for (int index = 0; index < m_numScales-1; ++index)
    {
        if (index == m_numScales - 1 || animationTime < m_scales[index + 1].timeStamp)
            return index;
    }
    assert(0);
}

static void Lerp(const __m128& a, const __m128& b, const float t, __m128& out) {
    
    __m128 oneMinusT = _mm_set1_ps(1.0f - t);
    __m128 resultA = _mm_mul_ps(a, oneMinusT);
    __m128 resultB = _mm_mul_ps(b, _mm_set1_ps(t));
    out = _mm_add_ps(resultA, resultB);
}

static inline __m128 lincomb_SSE(const __m128 &a, const __m128 B[4])
{
    __m128 result;
    result = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x00), B[0]);
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0x55), B[1]));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xaa), B[2]));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xff), B[3]));
    return result;
}

// this is the right approach for SSE ... SSE4.2
void matmult_SSE( __m128 out[4],  __m128 const A[4],  __m128 const B[4])
{
    // out_ij = sum_k a_ik b_kj
    // => out_0j = a_00 * b_0j + a_01 * b_1j + a_02 * b_2j + a_03 * b_3j
    __m128 out0x = lincomb_SSE(A[0], B);
    __m128 out1x = lincomb_SSE(A[1], B);
    __m128 out2x = lincomb_SSE(A[2], B);
    __m128 out3x = lincomb_SSE(A[3], B);

    out[0] = out0x;
    out[1] = out1x;
    out[2] = out2x;
    out[3] = out3x;
}

void CopyMat4ToSimd(const glm::mat4& matrix, __m128* simdData) {
    const float* matrixData = reinterpret_cast<const float*>(&matrix);
    for (int i = 0; i < 4; ++i) {
        simdData[i] = _mm_load_ps(&matrixData[i * 4]); // Aligned load
    }
}

void CopyVec4ToSimd(const glm::vec4& vector, __m128& simdData) {
    const float* vectorData = reinterpret_cast<const float*>(&vector);
    simdData = _mm_load_ps(vectorData);
}


void Bone::Update(float animationTime)
{
    glm::mat4  translation = InterpolatePosition(animationTime);
    glm::mat4  rotation =  InterpolateRotation(animationTime);
    
    __m128 a_simd[4];
    CopyMat4ToSimd(translation, a_simd);
    __m128 b_simd[4];
    CopyMat4ToSimd(rotation, b_simd);
    __m128 out[4];
    matmult_SSE(out, b_simd, a_simd);
    std::memcpy(&m_localTransform[0][0], &out[0], sizeof(glm_vec4) * 4);
    
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float scaleFactor = 0.0f;
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

glm::mat4 Bone::InterpolatePosition(float animationTime)
{
    if (1 == m_numPositions)
        return glm::translate(glm::mat4(1.0f), glm::vec3(m_positons[0].position));

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_positons[p0Index].timeStamp,
                                       m_positons[p1Index].timeStamp, animationTime);

    __m128 a_simd;
    __m128 b_simd;
    CopyVec4ToSimd(m_positons[p0Index].position, a_simd);
    CopyVec4ToSimd(m_positons[p1Index].position, b_simd);
    __m128 out;
    Lerp(a_simd, b_simd, scaleFactor, out);
    glm::vec4 finalPosition;
    std::memcpy(&finalPosition, &out, sizeof(__m128));
   // glm::vec4 finalPosition = glm::mix(m_positons[p0Index].position, m_positons[p1Index].position, scaleFactor);
    return glm::translate(glm::mat4(1.0f), glm::vec3(finalPosition));
}

glm::mat4 Bone::InterpolateRotation(float animationTime)
{
    if (1 == m_numRotations)
    {
        auto rotation = glm::normalize(m_rotations[0].orientation);
        return glm::mat4_cast(rotation);
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_rotations[p0Index].timeStamp, m_rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_rotations[p0Index].orientation, m_rotations[p1Index].orientation,
                                         scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::mat4_cast(finalRotation);
}

glm::mat4 Bone::InterpolateScaling(float animationTime)
{
    if (1 == m_numScales)
        return glm::scale(glm::mat4(1.0f), m_scales[0].scale);

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_scales[p0Index].timeStamp,
                                       m_scales[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(m_scales[p0Index].scale, m_scales[p1Index].scale
                                    , scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

Animation::Animation(const std::string& animationPath, ModelLoader* model)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    auto animation = scene->mAnimations[0];
    m_Duration = floor(animation->mDuration);
    m_TicksPerSecond = animation->mTicksPerSecond;
    ReadHeirarchyData(m_RootNode, scene->mRootNode);
    ReadMissingBonesIfAny(animation, *model);
}

Bone* Animation::FindBone(const size_t& name)
{
    if(m_Bones.count(name) > 0)
    {
        return &m_Bones[name];
    }
    return nullptr;
}


void Animation::ReadMissingBonesIfAny(const aiAnimation* animation, ModelLoader& model)
{
    int size = animation->mNumChannels;

    auto& boneInfoMap = model.GetBoneInfoMap();
    int& boneCount = model.GetBoneCount();

    //reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        auto boneNameHash = std::hash<std::string>{}(channel->mNodeName.data);

        if (boneInfoMap.find(boneNameHash) == boneInfoMap.end())
        {
            boneInfoMap[boneNameHash].id = boneCount;
            boneCount++;
        }
        m_Bones[boneNameHash] = Bone(channel->mNodeName.data, boneInfoMap[boneNameHash].id, channel);
    }

    m_BoneInfoMap = boneInfoMap;
}

void Animation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
{
    assert(src);
    dest.hashValue = std::hash<std::string>{}(src->mName.data);
    dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;
    for (int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        ReadHeirarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

Animator::Animator(Animation* animation, SceneObject* target, float startOffset)
{
    m_DeltaTime = 0.0;
    m_CurrentTime = 0.0 + startOffset;
    m_CurrentAnimation = animation;
    m_skinnedMesh = target;
    m_boneUbo = -1;

    m_FinalBoneMatrices.reserve(100);

    for (int i = 0; i < 100; i++)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    auto nameHash = node->hashValue;
    glm::mat4 nodeTransform = node->transformation;
    Bone* Bone = m_CurrentAnimation->FindBone(nameHash);
    if (Bone)
    {
        Bone->Update(m_CurrentTime);
        nodeTransform = Bone->GetLocalTransform();
    }
    glm::mat4 globalTransformation;
    __m128 a_simd[4];
    CopyMat4ToSimd(parentTransform, a_simd);
    __m128 b_simd[4];
    CopyMat4ToSimd(nodeTransform, b_simd);
    __m128 out[4];
    matmult_SSE(out, b_simd, a_simd);
    std::memcpy(&globalTransformation[0][0], &out[0], sizeof(glm_vec4) * 4);
    
    auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nameHash) != boneInfoMap.end())
    {
        int index = boneInfoMap[nameHash].id;
        glm::mat4 offset = boneInfoMap[nameHash].offset;
        CopyMat4ToSimd(globalTransformation, a_simd);
        CopyMat4ToSimd(offset, b_simd);
        matmult_SSE(out, b_simd, a_simd);
        std::memcpy(&m_FinalBoneMatrices[index][0][0], &out[0], sizeof(glm_vec4) * 4);
    }   
    
	
    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(&node->children[i], globalTransformation);
}

void Animator::UpdateAnimation(float dt)
{
    m_DeltaTime = dt;
    if (m_CurrentAnimation)
    {
        m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
        CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
    }
    UpdateRendererFlags(m_skinnedMesh);
}

void Animator::UpdateMatricesCache()
{
    m_FinalBoneMatricesCached = m_FinalBoneMatrices;
}


void Animator::UpdateUboMatrices()
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_boneUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 100 * sizeof(glm::mat4), m_FinalBoneMatricesCached.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}



void Animator::UpdateRendererFlags(SceneObject* object)
{
    object->renderer->IsSkinnedMesh = true;
    object->animator = this;
    object->renderer->GetMainMaterial()->SetInt("_isSkinnedMesh",1);
    for(int i = 0; i < object->childs.size();i++)
    {
        UpdateRendererFlags(object->childs[i].get());
    }
}

const std::vector<glm::mat4>& Animator::GetFinalBoneMatrices()
{
    return m_FinalBoneMatrices;
}





