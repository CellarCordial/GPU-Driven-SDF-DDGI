#ifndef RENDER_PASS_SHADOW_MAP_H
#define RENDER_PASS_SHADOW_MAP_H

#include "../../render_graph/render_pass.h"
#include "../../core/math/matrix.h"
#include "../../scene/geometry.h"
#include <memory>

namespace fantasy 
{
    namespace constant
    {
        struct ShadowMapPassConstant
        {
            float4x4 world_matrix = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
            float4x4 directional_light_view_proj;
        };
    }


    class ShadowMapPass : public RenderPassInterface
    {
    public:
        ShadowMapPass() { type = RenderPassType::Graphics; }
        
        bool compile(DeviceInterface* device, RenderResourceCache* cache) override;
        bool execute(CommandListInterface* cmdlist, RenderResourceCache* cache) override;
        bool finish_pass(RenderResourceCache* cache) override;

		friend class AtmosphereTest;

    private:
        bool _resource_writed = false;
        std::vector<Vertex> _vertices;
        std::vector<uint32_t> _indices;
        constant::ShadowMapPassConstant _pass_constant;

		std::shared_ptr<BufferInterface> _vertex_buffer;
		std::shared_ptr<BufferInterface> _index_buffer;
        std::shared_ptr<TextureInterface> _shadow_map_texture;

        std::shared_ptr<BindingLayoutInterface> _binding_layout;
        std::shared_ptr<InputLayoutInterface> _input_layout;

		std::shared_ptr<Shader> _vs;
		std::shared_ptr<Shader> _ps;

        std::unique_ptr<FrameBufferInterface> _frame_buffer;
        std::unique_ptr<GraphicsPipelineInterface> _pipeline;
        
        GraphicsState _graphics_state;

        std::vector<DrawArguments> _draw_arguments;
    };

}












#endif