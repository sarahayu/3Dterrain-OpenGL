#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform bool debug;

uniform sampler2D screenTexture;

void main()
{

    float blur = 0.1;

    float resolution = blur * 0.1;
    int n = 10;
    int i = -n;
    int totalWeight = 0;

    vec2 st = TexCoords;

    vec4 outColor = vec4(0.0);
    for(; i < n; ++i) {
        vec2 uv = st + vec2(float(i) / 2.0 / float(n) * resolution, 0);
        outColor += vec4(texture(screenTexture, uv).rgb,1.0);
        totalWeight += 1;
    }
    FragColor = outColor / float(totalWeight);

    //vec3 col = (texture(screenTexture, TexCoords).rgb
				//	+ texture(screenTexture, TexCoords + vec2(0.0, -0.001)).rgb
				//	+ texture(screenTexture, TexCoords + vec2(0.0, 0.001)).rgb) / 3;
    //FragColor = vec4(col, 1.0);
} 