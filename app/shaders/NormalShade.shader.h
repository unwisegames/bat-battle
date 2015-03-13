#ifdef BRICABRAC_VERTEX_SHADER

BRICABRAC_UNIFORM(mat4, pmv)

BRICABRAC_ATTRIBUTE(vec2, position)
BRICABRAC_ATTRIBUTE(vec2, texcoord)

#ifndef BRICABRAC_HOSTED

varying vec2 v_texcoord;

void main() {
    v_texcoord = texcoord;
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

void main() {
    mediump vec4 tex = texture2D(texture, v_texcoord);
    mediump vec3 normal = 2.0 * tex.xyz - 1.0;
    gl_FragColor = vec4(tint * (0.75 + 0.25 * dot(light, normal)), 1.0) * tex.a;
}

#endif // BRICABRAC_HOSTED
#endif // BRICABRAC_FRAGMENT_SHADER
