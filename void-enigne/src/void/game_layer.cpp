#include "game_layer.h"
#include "renderer.h"
#include "resource_system.h"
#include "profiler.h"


namespace VoidEngine
{
    void GameLayer::OnAttach()
    {
        SIMPLE_LOG("attach!");
    }

    void GameLayer::OnUpdate(double dt)
    {     
        Renderer::NewFrame();
        Renderer::Draw(mesh, material);
        Renderer::EndFrame();
    }

    void GameLayer::OnDetach()
    {
        SIMPLE_LOG("detach!");
    }

    void GameLayer::OnInit()
    {
        Vertex quadVertices[] =
        {
            //   position (x, y, z, w)       uv
            { { -1.0f,  1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }, // top-left
            { {  1.0f,  1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }, // top-right
            { {  1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } }, // bottom-right
            { { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }  // bottom-left
        };

        uint32_t quadIndices[] =
        {
            0, 1, 2,
            0, 2, 3
        };

        mesh = ResourceSystem::Create<MeshResource>(123, false);
        mesh->SetVertexData(quadVertices, 4);
        mesh->SetIndexData(quadIndices, 6);
        mesh->SubmitMeshToGpu();

        auto shader = ResourceSystem::Load<ShaderResource>(L"src//asset//shader//square_demo.hlsl");
        material = ResourceSystem::Create<MaterialResource>(ResourceSystem::GenerateGUID(), shader->GetGUID());

        ECS::Entity e = world.CreateEntity();
        std::cout << "Entity id: " << e.GetId() << " , gen count: " << e.GetGenCount() << std::endl;
        e.Destroy();
        e = world.CreateEntity();
        std::cout << "Entity id: " << e.GetId() << " , gen count: " << e.GetGenCount() << std::endl;
        
        world.Register<Position>();

        e.Add<Position>({1,1});
    }   

    void GameLayer::OnEvent(const Event& e)
    {
        switch(e.GetEventType())
        {

            case EventType::KEY_PRESSED:
            {
                std::cout << "Key Pressed Game Layer" << std::endl;
                break;
            }
            case EventType::KEY_RELEASED:
            {
                std::cout << "Key Released Game Layer" << std::endl;
                break;
            }
        }
    }
}