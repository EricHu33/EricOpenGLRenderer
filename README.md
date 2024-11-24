# EricOpenGLRenderer

A OpenGL renderer include below features:
 - Forward Shading, Deferred Shading
 - Forward+ Rendering (Using Compute shader to cull lights by tiles, store lights into one SSBO) 
 - PBR (Mostly from google filament, also inspired some algo from Unity/UE)
 - Cloth PBR (Unity HDRP, google filament)
 - Skin PBR (Using The Order 1886's method, preintegrated SSS)
 - Cascaded Shadow Map (improved version of LearnOpenGL, no edge swimming issue)
 - PCSS shadow (soft shadow)
 - Physics Based Light Unit (Google filement)
 - Using Spherical Harmonics for indirect light source(Thanks to MJP's BakingLab repo)
 - FXAA (NVDA paper)
 - HBAO (NVDA paper)
 - SSAO
 - RSM(does not perform well so disable by default)
 - Skinned Animation (improved version of LearnOpenGL, with faster bone-mapping speed during skinning and overall faster execution time by SSE/AVX and multithreading)
 - imgui and some scene widget

## Demo scenes

- "Scenes/PBRClothDemoScene"
- "Scenes/PBRPropsDemoScene"
- "Scenes/PBRPSkinnedDemoScene"
- "Scenes/PBRSphereDemoScene"
- "Scenes/SponzaDemoScene"

![{7BD0A74F-48A1-41D8-8617-381CB3D2DF97}](https://github.com/user-attachments/assets/1542766a-6ea1-4a2d-9d57-1d11cb271eea)
![{22F64F13-8F7E-42FC-9419-BB3D919412A2}](https://github.com/user-attachments/assets/6e5e56f9-27c1-4ed5-be4f-5936913b941c)
![{08A22D2B-193F-4249-96AE-52F2C2DB422C}](https://github.com/user-attachments/assets/edca990a-650e-4c4e-878f-7d5d187c0fd6)
![{54EBFE78-18FC-428B-B03E-36663F5B6013}](https://github.com/user-attachments/assets/18f28f49-80c3-45b5-a28e-554d48800759)
![{3E9C62E2-9D7B-4F04-865C-2080839AD357}](https://github.com/user-attachments/assets/eb640781-dd88-44e6-92bd-5748e047382a)
