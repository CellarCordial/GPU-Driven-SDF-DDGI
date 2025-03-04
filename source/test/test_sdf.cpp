#include "test_sdf.h"
#include "../shader/shader_compiler.h"
#include "../core/tools/check_cast.h"
#include "../scene/camera.h"
#include <memory>

namespace fantasy
{
#define SDF_RESOLUTION 128

	bool SdfDebugPass::compile(DeviceInterface* device, RenderResourceCache* cache)
	{
		// Binding Layout.
		{
			BindingLayoutItemArray binding_layout_items(3);
			binding_layout_items[0] = BindingLayoutItem::create_push_constants(0, sizeof(constant::SdfDebugPassConstants));
			binding_layout_items[1] = BindingLayoutItem::create_texture_srv(0);
			binding_layout_items[2] = BindingLayoutItem::create_sampler(0);
			ReturnIfFalse(_binding_layout = std::unique_ptr<BindingLayoutInterface>(device->create_binding_layout(
				BindingLayoutDesc{ .binding_layout_items = binding_layout_items }
			)));
		}

		// Shader.
		{
			ShaderCompileDesc ShaderCompileDesc;
			ShaderCompileDesc.shader_name = "common/full_screen_quad_vs.hlsl";
			ShaderCompileDesc.entry_point = "main";
			ShaderCompileDesc.target = ShaderTarget::Vertex;
			ShaderData vs_data = compile_shader(ShaderCompileDesc);
			ShaderCompileDesc.shader_name = "test/sdf_test_ps.hlsl";
			ShaderCompileDesc.entry_point = "main";
			ShaderCompileDesc.target = ShaderTarget::Pixel;
			ShaderData ps_data = compile_shader(ShaderCompileDesc);

			ShaderDesc shader_desc;
			shader_desc.shader_type = ShaderType::Vertex;
			shader_desc.entry = "main";
			_vs = std::unique_ptr<Shader>(create_shader(shader_desc, vs_data.data(), vs_data.size()));

			shader_desc.shader_type = ShaderType::Pixel;
			shader_desc.entry = "main";
			_ps = std::unique_ptr<Shader>(create_shader(shader_desc, ps_data.data(), ps_data.size()));
		}

		// Frame Buffer.
		{
			FrameBufferDesc FrameBufferDesc;
			FrameBufferDesc.color_attachments.push_back(FrameBufferAttachment::create_attachment(
				check_cast<TextureInterface>(cache->require("final_texture"))
			));
			ReturnIfFalse(_frame_buffer = std::unique_ptr<FrameBufferInterface>(device->create_frame_buffer(FrameBufferDesc)));
		}

		// Pipeline.
		{
			GraphicsPipelineDesc PipelineDesc;
			PipelineDesc.vertex_shader = _vs;
			PipelineDesc.pixel_shader = _ps;
			PipelineDesc.binding_layouts.push_back(_binding_layout);
			ReturnIfFalse(_pipeline = std::unique_ptr<GraphicsPipelineInterface>(device->create_graphics_pipeline(
				PipelineDesc, 
				_frame_buffer.get()
			)));
		}

		// Binding Set.
		{
			_sdf_texture = check_cast<TextureInterface>(cache->require("GlobalSdfTexture"));
			
			BindingSetItemArray binding_set_items(3);
			binding_set_items[0] = BindingSetItem::create_push_constants(0, sizeof(constant::SdfDebugPassConstants));
			binding_set_items[1] = BindingSetItem::create_texture_srv(0, _sdf_texture);
			binding_set_items[2] = BindingSetItem::create_sampler(0, check_cast<SamplerInterface>(cache->require("linear_clamp_sampler")));
			ReturnIfFalse(_binding_set = std::unique_ptr<BindingSetInterface>(device->create_binding_set(
				BindingSetDesc{ .binding_items = binding_set_items },
				_binding_layout
			)));
		}

		// Graphics state.
		{
			_graphics_state.pipeline = _pipeline.get();
			_graphics_state.frame_buffer = _frame_buffer.get();
			_graphics_state.binding_sets.push_back(_binding_set.get());
			_graphics_state.viewport_state = ViewportState::create_default_viewport(CLIENT_WIDTH, CLIENT_HEIGHT);
		}

		float chunk_size = (SDF_SCENE_GRID_SIZE / GLOBAL_SDF_RESOLUTION) * VOXEL_NUM_PER_CHUNK;
		_global_sdf_data.default_march = chunk_size;
		_global_sdf_data.sdf_grid_size = SDF_SCENE_GRID_SIZE;
		_global_sdf_data.sdf_grid_origin = float3(-SDF_SCENE_GRID_SIZE * 0.5f);

		ReturnIfFalse(cache->collect_constants("GlobalSDFInfo", &_global_sdf_data));

		return true;
	}
	bool SdfDebugPass::execute(CommandListInterface* cmdlist, RenderResourceCache* cache)
	{
		// Update constant.
		{
			ReturnIfFalse(cache->get_world()->each<Camera>(
				[this](Entity* entity, Camera* camera) -> bool
				{
					Camera::FrustumDirections Directions = camera->get_frustum_directions();
					_pass_constants.frustum_a = Directions.A;
					_pass_constants.frustum_b = Directions.B;
					_pass_constants.frustum_c = Directions.C;
					_pass_constants.frustum_d = Directions.D;
					_pass_constants.camera_position = camera->position;
					return true;
				}
			));

			_pass_constants.sdf_data = _global_sdf_data;
		}
		
		ReturnIfFalse(cmdlist->open());

		ReturnIfFalse(cmdlist->draw(_graphics_state, DrawArguments{ .index_count = 6 }, &_pass_constants));

		ReturnIfFalse(cmdlist->close());
		return true;
	}

	bool SdfTest::setup(RenderGraph* render_graph)
	{
		ReturnIfFalse(render_graph != nullptr);

		_sdf_generate_pass = std::make_shared<SdfGeneratePass>();
		_global_sdf_pass = std::make_shared<GlobalSdfPass>();
		sdf_debug_pass = std::make_shared<SdfDebugPass>();

		render_graph->add_pass(_sdf_generate_pass);
		render_graph->add_pass(_global_sdf_pass);
		render_graph->add_pass(sdf_debug_pass);

		return true;
	}

}