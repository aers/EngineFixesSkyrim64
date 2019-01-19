### Specular Lighting Bug

Note: I learned everything I know about shaders and lighting in one day so I might mis-use some terms here, but it should be at least understandable for an experienced graphics programmer...

Skyrim SE uses the [Blinn-Phong](https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_shading_model) model to do specular lighting. This is a very simple model that looks something like this:

```
inputs: lightDirection, viewDirection, normal

half = normalize(lightDirection + viewDirection)
NdotH = dot(normal, half)

specular = <calculation using NdotH>
```
In SSE, normals are stored as model-space normals in a _msn texture file. To convert those model-space normals into the proper space for the above calculations, the MSN is transformed using a model-view matrix:

```
inputs: lightDirection, viewDirection, modelViewMat

msnormal = read_from_texture

normal = msnormal * modelViewMat

half = normalize(lightDirection + viewDirection)
NdotH = dot(normal, half)

specular = <calculation using NdotH>
```
This part is actually done entirely correctly in the shader. The issue is that there's some extra stuff going on that breaks it.

This is what SE's specular calculation actually looks like:

```
inputs: lightDirection, viewDirection, modelViewMat

viewDirectionTransformed = viewDirection * modelViewMat

msnormal = read_from_texture

normal = msnormal * modelViewMat

half = normalize(lightDirection + viewDirectionTransformed)
NdotH = dot(normal, half)

specular = <calculation using NdotH>
```
For some reason (probably just a mistake from hastily written code, or even a typo) the view is being transformed by the model-view matrix before being fed into the rest of the calculatons. This ends up basically negating the transformation of the MSN, so all the normals end up pointing north - and every light is considered behind the actor unless the actor's mesh is facing north. 

The fix is simple - just get rid of that view transform and use the actual view vector for calculations. I have no way of knowing what the BSLightingShader  source code actually looks like so I can't help you there :)

In the binary shaders we have access to, the broken code is actually right at the start of the shader. Here's an example:

```
// v6.xyz = view direction vector
// v3, v4, v5 = model-view matrix
dp3 r0.x, v3.xyzx, v6.xyzx   // transform v6 by v3/v4/v5
dp3 r0.y, v4.xyzx, v6.xyzx
dp3 r0.z, v5.xyzx, v6.xyzx
dp3 r0.w, r0.xyzx, r0.xyzx  // normalize
rsq r0.w, r0.w
mul r1.xyz, r0.wwww, r0.xyzx // store transformed view in r1.xyz
// this incorrect view is then used in future specular calculations like so
mad r11.xyz, r0.xyzx, r0.wwww, r11.xyzx // r11.xyz = r0.xyz*r0.www + r11.xyz <-- this is lightDirection + viewDirection using the transformed view
```

My patch is very simple, I patched every binary shader that matched the pattern of transforming the view to be do this instead:

```
// v6.xyz = view direction vector
// v3, v4, v5 = model-view matrix
// patched
mov r0.xyz, r6.xyz // put v6 in v0 instead of transforming it into v0
nop * 16           // fill in now useless instruction space with nops
// unpatched from here on
dp3 r0.w, r0.xyzx, r0.xyzx  // normalize
rsq r0.w, r0.w
mul r1.xyz, r0.wwww, r0.xyzx // store regular view in r1

etc
```

Since we're looking at compiled shaders, that was 378 total. Obviously with access to the shader source this is far easier to fix :)

PS: There's actually a few other bugs in the shaders (LOD reflections in water are broken and display only as black boxes, for example) but its too difficult for us to track those down without the shader source. We got really lucky on this bug. I don't really know if its feasible but having access to the shader source would be a modder's dream.

