## Terrain Light Map Baking
> aka Burn Terrain

WorldEdit now supports baking the terrain's light map. While not as capable as it could be it should be fairly competitive with Zero Editor's "Burn Terrain" function.

The baking is performed by evaluating the ambient lighting (with optional ambient occlusion) and any lights marked static at each terrain grid point. If Supersample is enabled multiple samples will be taken around the terrain grid point and averaged together.

Ambient occlusion and shadows for lights are calculated with simple ray tracing.

The results seem generally comparable to ZE's (excluding ambient occlusion which it doesn't do) but do not match exactly.

Near instant bake times can be achieved for common terrain sizes by lowering the Ambient Occlusion sample count and turning off Supersample. 

## Limitations

- No support for directional region lights.
- No support for materials of any kind when ray tracing shadows.


## Future Work

- Least Squares Vertex Baking - https://cal.cs.umbc.edu/Papers/Kavan-2011-LSV/
- Path Tracing lighting to gather indirect light as well as direct light.
- Baking on the GPU.