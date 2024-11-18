#include "shader_list.hpp"

namespace we::graphics {

namespace shaders {

auto mesh_shadowVS() noexcept -> shader_def;
auto mesh_shadow_cutoutVS() noexcept -> shader_def;
auto mesh_shadow_cutoutPS() noexcept -> shader_def;
auto meshVS() noexcept -> shader_def;
auto mesh_basicPS() noexcept -> shader_def;
auto mesh_basic_lightingPS() noexcept -> shader_def;
auto mesh_normalPS() noexcept -> shader_def;
auto mesh_depth_prepassVS() noexcept -> shader_def;
auto mesh_depth_cutoutPS() noexcept -> shader_def;
auto mesh_wireframeVS() noexcept -> shader_def;
auto mesh_wireframePS() noexcept -> shader_def;
auto mesh_wireframe_GS_fallbackPS() noexcept -> shader_def;
auto mesh_wireframe_GS_fallbackGS() noexcept -> shader_def;
auto terrain_patchVS() noexcept -> shader_def;
auto terrain_basicPS() noexcept -> shader_def;
auto terrain_lightingPS() noexcept -> shader_def;
auto terrain_normalPS() noexcept -> shader_def;
auto terrain_gridPS() noexcept -> shader_def;
auto terrain_foliage_mapPS() noexcept -> shader_def;
auto terrain_cut_mesh_clearVS() noexcept -> shader_def;
auto sky_meshVS() noexcept -> shader_def;
auto sky_meshPS() noexcept -> shader_def;
auto waterVS() noexcept -> shader_def;
auto waterPS() noexcept -> shader_def;
auto thumbnail_meshVS() noexcept -> shader_def;
auto thumbnail_meshPS() noexcept -> shader_def;
auto thumbnail_mesh_alpha_cutoutPS() noexcept -> shader_def;
auto thumbnail_downsampleVS() noexcept -> shader_def;
auto thumbnail_downsamplePS() noexcept -> shader_def;
auto resample_env_mapVS() noexcept -> shader_def;
auto resample_env_mapPS() noexcept -> shader_def;
auto meta_drawPS() noexcept -> shader_def;
auto meta_draw_wireframePS() noexcept -> shader_def;
auto meta_draw_wireframe_GS_fallbackPS() noexcept -> shader_def;
auto meta_draw_wireframe_GS_fallbackGS() noexcept -> shader_def;
auto meta_draw_outlinedPS() noexcept -> shader_def;
auto meta_draw_outlined_GS_fallbackPS() noexcept -> shader_def;
auto meta_draw_outlined_GS_fallbackGS() noexcept -> shader_def;
auto meta_draw_primitiveVS() noexcept -> shader_def;
auto meta_draw_shape_outlinedVS() noexcept -> shader_def;
auto meta_draw_shapeVS() noexcept -> shader_def;
auto meta_draw_sphereVS() noexcept -> shader_def;
auto meta_draw_linePS() noexcept -> shader_def;
auto meta_draw_lineVS() noexcept -> shader_def;
auto ai_overlay_shapeVS() noexcept -> shader_def;
auto ai_overlay_applyVS() noexcept -> shader_def;
auto ai_overlay_applyPS() noexcept -> shader_def;
auto tile_lights_clearCS() noexcept -> shader_def;
auto depth_reduce_minmaxCS() noexcept -> shader_def;
auto grid_overlayVS() noexcept -> shader_def;
auto grid_overlayPS() noexcept -> shader_def;
auto gizmo_coneVS() noexcept -> shader_def;
auto gizmo_conePS() noexcept -> shader_def;
auto tile_lightsVS() noexcept -> shader_def;
auto tile_lightsPS() noexcept -> shader_def;
auto imguiVS() noexcept -> shader_def;
auto imguiPS() noexcept -> shader_def;

}

std::initializer_list<shader_def> shader_list = {
shaders::mesh_shadowVS(),
shaders::mesh_shadow_cutoutVS(),
shaders::mesh_shadow_cutoutPS(),
shaders::meshVS(),
shaders::mesh_basicPS(),
shaders::mesh_basic_lightingPS(),
shaders::mesh_normalPS(),
shaders::mesh_depth_prepassVS(),
shaders::mesh_depth_cutoutPS(),
shaders::mesh_wireframeVS(),
shaders::mesh_wireframePS(),
shaders::mesh_wireframe_GS_fallbackPS(),
shaders::mesh_wireframe_GS_fallbackGS(),
shaders::terrain_patchVS(),
shaders::terrain_basicPS(),
shaders::terrain_lightingPS(),
shaders::terrain_normalPS(),
shaders::terrain_gridPS(),
shaders::terrain_foliage_mapPS(),
shaders::terrain_cut_mesh_clearVS(),
shaders::sky_meshVS(),
shaders::sky_meshPS(),
shaders::waterVS(),
shaders::waterPS(),
shaders::thumbnail_meshVS(),
shaders::thumbnail_meshPS(),
shaders::thumbnail_mesh_alpha_cutoutPS(),
shaders::thumbnail_downsampleVS(),
shaders::thumbnail_downsamplePS(),
shaders::resample_env_mapVS(),
shaders::resample_env_mapPS(),
shaders::meta_drawPS(),
shaders::meta_draw_wireframePS(),
shaders::meta_draw_wireframe_GS_fallbackPS(),
shaders::meta_draw_wireframe_GS_fallbackGS(),
shaders::meta_draw_outlinedPS(),
shaders::meta_draw_outlined_GS_fallbackPS(),
shaders::meta_draw_outlined_GS_fallbackGS(),
shaders::meta_draw_primitiveVS(),
shaders::meta_draw_shape_outlinedVS(),
shaders::meta_draw_shapeVS(),
shaders::meta_draw_sphereVS(),
shaders::meta_draw_linePS(),
shaders::meta_draw_lineVS(),
shaders::ai_overlay_shapeVS(),
shaders::ai_overlay_applyVS(),
shaders::ai_overlay_applyPS(),
shaders::tile_lights_clearCS(),
shaders::depth_reduce_minmaxCS(),
shaders::grid_overlayVS(),
shaders::grid_overlayPS(),
shaders::gizmo_coneVS(),
shaders::gizmo_conePS(),
shaders::tile_lightsVS(),
shaders::tile_lightsPS(),
shaders::imguiVS(),
shaders::imguiPS(),
};

}