#pragma once
#include "layer.h"
#include "resource.h"

namespace VoidEngine
{
    class GameLayer : public Layer
    {
    public:
        GameLayer():
            m_gameTime(0)
        {
        }

        void OnInit() override;
        void OnDetach() override;
        void OnAttach() override;
        void OnUpdate(double dt) override;
        void OnEvent(const Event& e) override;

    private:
        size_t m_gameTime;
        MeshResource* mesh;
        MaterialResource* material;
    };
}