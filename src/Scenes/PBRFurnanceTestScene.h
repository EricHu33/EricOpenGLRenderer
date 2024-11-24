#pragma once
#include "imgui.h"
#include "../Scene.h"
#include "../DebugConfig.h"
namespace EricScene
{
    struct PBRFurnanceTestScene : Scene
    {
        void CreateScene() override;
        void Update(float dt) override;
        void DeleteScene() override;
        float rotateTheta = 0;
        virtual void OnGUI() override
        {
        }
    };
}