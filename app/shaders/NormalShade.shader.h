#ifdef BRICABRAC_VERTEX_SHADER

BRICABRAC_UNIFORM(mat4, pmv)

BRICABRAC_ATTRIBUTE(vec2, position)
BRICABRAC_ATTRIBUTE(vec2, texcoord)
//BRICABRAC_ATTRIBUTE(float, distance)

#ifndef BRICABRAC_HOSTED

varying vec2 v_texcoord;
//varying float v_alpha;

void main() {
    v_texcoord = texcoord;
    //v_alpha = 1.0 - smoothstep(0.0, 5.0, distance);
    gl_Position = pmv * vec4(position, 0, 1);
}

#endif // BRICABRAC_HOSTED
#endif // BRICABRAC_VERTEX_SHADER

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifdef BRICABRAC_FRAGMENT_SHADER

BRICABRAC_UNIFORM(mediump vec3, light)
BRICABRAC_UNIFORM(mediump vec3, tint)
BRICABRAC_UNIFORM(sampler2D, texture)

#ifndef BRICABRAC_HOSTED

varying mediump vec2 v_texcoord;
//varying mediump float v_alpha;

void main() {
    mediump vec4 tex = texture2D(texture, v_texcoord);
    mediump vec3 normal = 2.0 * tex.xyz - 1.0;
    gl_FragColor = vec4(tint, 1.0) * tex;// * v_alpha;
}

#endif // BRICABRAC_HOSTED
#endif // BRICABRAC_FRAGMENT_SHADER
