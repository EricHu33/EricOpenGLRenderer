#pragma once
#include <map>
#include <vector>
#include "ModelLoader.h"
#include <glm/gtx/transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <robin_hood/robin_hood.h>

struct SceneObject;

struct KeyPosition
{
    glm::vec4 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

class Bone
{
private:
    std::vector<KeyPosition> m_positons;
    std::vector<KeyRotation> m_rotations;
    std::vector<KeyScale> m_scales;
    int m_numPositions;
    int m_numRotations;
    int m_numScales;
    glm::mat4 m_localTransform;
    std::string m_name;
    int m_ID;
public:
    Bone() = default;
    Bone(const std::string& name, int ID, const aiNodeAnim* channel);

    void Update(float animationTime);
    glm::mat4& GetLocalTransform() { return m_localTransform; }
    std::string GetBoneName() const { return m_name; }
    int GetBoneID() { return m_ID; }
    int GetPositionIndex(float animationTime);
    int GetRotationIndex(float animationTime);
    int GetScaleIndex(float animationTime);

private:
    /* Gets normalized value for Lerp & Slerp*/
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
    glm::mat4 InterpolatePosition(float animationTime);
    glm::mat4 InterpolateRotation(float animationTime);
    glm::mat4 InterpolateScaling(float animationTime);
};

struct AssimpNodeData
{
    glm::mat4 transformation;
    size_t hashValue;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
private:
    float m_Duration;
    int m_TicksPerSecond;
    robin_hood::unordered_map<size_t, Bone> m_Bones;
    AssimpNodeData m_RootNode;
    robin_hood::unordered_map<size_t, BoneInfo> m_BoneInfoMap;
public:
    Animation() = default;
    Animation(const std::string& animationPath, ModelLoader* model);
    ~Animation(){}
    
    Bone* FindBone(const size_t& name);
    inline float GetTicksPerSecond()                              { return m_TicksPerSecond; }
    inline float GetDuration()                                    { return m_Duration;}
    inline const AssimpNodeData& GetRootNode()                    { return m_RootNode; }
    robin_hood::unordered_map<size_t,BoneInfo>& GetBoneIDMap()   { return m_BoneInfoMap;}
private:
    void ReadMissingBonesIfAny(const aiAnimation* animation, ModelLoader& model);
    void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);
};

class Animator
{
public:
    Animator(Animation* animation, SceneObject* target, float startOffset);
    void UpdateAnimation(float dt);
    const std::vector<glm::mat4>& GetFinalBoneMatrices();
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
    void UpdateRendererFlags(SceneObject* object);
    void UpdateMatricesCache();
    void UpdateUboMatrices();
    void SetUbo(unsigned int ubo){ m_boneUbo = ubo; }
private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    std::vector<glm::mat4> m_FinalBoneMatricesCached;
    Animation* m_CurrentAnimation;
    SceneObject* m_skinnedMesh;
    unsigned int m_boneUbo = -1;
    float m_CurrentTime;
    float m_DeltaTime;
};

