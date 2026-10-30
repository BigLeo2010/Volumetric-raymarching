#version 430 core
out vec4 FragColor;

in vec3 pos;

uniform float time;
uniform vec3 camera_position;
uniform vec3 camForward; 
uniform vec3 camRight;
uniform vec3 camUp;
uniform sampler3D uNoise;
uniform sampler3D uNormal;
uniform float isoMin;
uniform float isoMax;
uniform vec3 rgbColorA;
uniform vec3 rgbColorB;

layout(binding = 2) uniform sampler2DArray stbnTexture;
uniform int frameIndex;  

uniform bool STBNsmooth;
uniform bool use_dvr;

uniform float clipX;
uniform float clipY;
uniform float clipZ;

const vec3 BOX_MIN = vec3(-2.0, -2.0, -2.0);
const vec3 BOX_MAX = vec3( 2.0,  2.0,  2.0);

bool rayAABBIntersection(vec3 ro, vec3 rd, vec3 boxMin, vec3 boxMax, out float tNear, out float tFar) {    
    vec3 tmin = (boxMin - ro) / rd;
    vec3 tmax = (boxMax - ro) / rd;
    
    vec3 t_1 = min(tmin, tmax);
    vec3 t_2 = max(tmin, tmax);
    
    tNear = max(max(t_1.x, t_1.y), t_1.z);
    tFar = min(min(t_2.x, t_2.y), t_2.z);
    
    return tNear < tFar && tFar > 0.0;
}

float hash(vec2 p) {
    return fract(52.9829189 * fract(dot(p, vec2(0.0605, 0.00767))));
}

float getSTBNNoise() {
    ivec2 pixelPos = ivec2(gl_FragCoord.xy) % 128;
    
    int layer = int(frameIndex % 64);
    
    return texelFetch(stbnTexture, ivec3(pixelPos, layer), 0).r;
}

float getVolumeDensity(vec3 uvw) {
    float d = texture(uNoise, uvw).r; 
    return d; 
}

vec3 getVolumeNormal(vec3 uvw) {
    vec3 encodedNormal = texture(uNormal, uvw).rgb;
    vec3 normal = encodedNormal * 2.0 - 1.0;
    return normalize(normal);
}

vec3 light_position = vec3(0.0, 5.0, 5.0);

vec3 getProceduralColor(float density)
{
    vec3 color = vec3(0.0);

    if (density > 0.05 && density <= 0.25) {
        float factor = (density - 0.05) / (0.25 - 0.05);
        color = mix(vec3(0.0, 0.05, 0.2), vec3(0.1, 0.3, 0.6), factor);
    }
    else if (density > 0.25 && density <= 0.45) {
        float factor = (density - 0.25) / (0.45 - 0.25);
        color = mix(vec3(0.4, 0.05, 0.05), vec3(0.7, 0.1, 0.1), factor);
    }
    else if (density > 0.45 && density <= 0.65) {
        float factor = (density - 0.45) / (0.65 - 0.45);
        color = mix(vec3(0.8, 0.1, 0.1), vec3(1.0, 0.4, 0.1), factor);
    }
    else if (density > 0.65) {
        float factor = clamp((density - 0.65) / (1.0 - 0.65), 0.0, 1.0);
        color = mix(vec3(0.9, 0.85, 0.8), vec3(1.0, 1.0, 1.0), factor);
    }

    return color;
}

vec3 ray_march(in vec3 ro, in vec3 rd)
{
    float tNear, tFar;

    if (!rayAABBIntersection(ro, rd, BOX_MIN, BOX_MAX, tNear, tFar)) {
        return vec3(35.0/255.0, 35.0/255.0, 35.0/255.0);
    }

    tNear = max(tNear, 0.0);
    
    float pathLength = tFar - tNear;
    const float NUM_OF_STEPS = 128;
    float STEP_SIZE = pathLength / float(NUM_OF_STEPS);
    
    float jitter;

    if (STBNsmooth) jitter = getSTBNNoise() * STEP_SIZE;
    else jitter = hash(gl_FragCoord.xy) * STEP_SIZE;

    float t = tNear + jitter;

	for (int i = 0; i < NUM_OF_STEPS; ++i)
    {
        if (t > tFar) break;

        vec3 p = ro + rd * t;

        if (p.x > clipX || p.y > clipY || p.z > clipZ) {
            t += STEP_SIZE;
            continue;
        }

        vec3 uvw = (p - BOX_MIN) / (BOX_MAX - BOX_MIN);

        float density = getVolumeDensity(uvw);

        if (density >= isoMin && density <= isoMax) {
            if (i > 0) {
                float prev_t = t - STEP_SIZE;
                vec3 prev_p = ro + rd * prev_t;
                vec3 prev_uvw = (prev_p - BOX_MIN) / (BOX_MAX - BOX_MIN);
                float prev_density = getVolumeDensity(prev_uvw);
                
                float deltaDensity = density - prev_density;
                if (abs(deltaDensity) > 1e-5) {
                    float factor = (isoMin - prev_density) / deltaDensity;
                    t = mix(prev_t, t, factor);
                    p = ro + rd * t;
                    uvw = (p - BOX_MIN) / (BOX_MAX - BOX_MIN);
                }
            }
            
            vec3 normal = getVolumeNormal(uvw);
            
            vec3 direction_to_light = normalize(light_position - p);

            float diffuse_intensity = max(0.0, dot(normal, direction_to_light));
            
            float ambient = 0.2; 
            float lighting = max(diffuse_intensity, ambient);

            vec4 baseColor = vec4(getProceduralColor(density), 0);
            
            return baseColor.rgb * lighting;
        }

        t += STEP_SIZE;
    }

    return vec3(35.0/255.0, 35.0/255.0, 35.0/255.0);
}

vec4 getProceduralColor_dvr(float density)
{
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    if (density > 0.05 && density <= 0.25) {
        float factor = (density - 0.05) / (0.25 - 0.05);
        color.rgb = mix(vec3(0.0, 0.05, 0.2), vec3(0.1, 0.3, 0.6), factor);
        color.a = 0.05;
    }
    
    else if (density > 0.25 && density <= 0.45) {
        float factor = (density - 0.25) / (0.45 - 0.25);
        color.rgb = mix(vec3(0.4, 0.05, 0.05), vec3(0.7, 0.1, 0.1), factor);
        color.a = 0.3;
    }
    
    else if (density > 0.45 && density <= 0.65) {
        float factor = (density - 0.45) / (0.65 - 0.45);
        color.rgb = mix(vec3(0.8, 0.1, 0.1), vec3(1.0, 0.4, 0.1), factor);
        color.a = 0.6;
    }
    
    else if (density > 0.65) {
        float factor = clamp((density - 0.65) / (1.0 - 0.65), 0.0, 1.0);
        color.rgb = mix(vec3(0.9, 0.85, 0.8), vec3(1.0, 1.0, 1.0), factor);
        color.a = 0.9;
    }

    return color;
}

vec3 ray_march_dvr(in vec3 ro, in vec3 rd)
{
    float tNear, tFar;
    vec3 backgroundColor = vec3(35.0 / 255.0, 35.0 / 255.0, 35.0 / 255.0);

    if (!rayAABBIntersection(ro, rd, BOX_MIN, BOX_MAX, tNear, tFar)) {
        return backgroundColor;
    }

    tNear = max(tNear, 0.0);
    
    float pathLength = tFar - tNear;
    const float NUM_OF_STEPS = 128;
    float STEP_SIZE = pathLength / float(NUM_OF_STEPS);
    
    float jitter;
    if (STBNsmooth) jitter = getSTBNNoise() * STEP_SIZE;
    else jitter = hash(gl_FragCoord.xy) * STEP_SIZE;

    float t = tNear + jitter;

    vec4 accumulatedColor = vec4(0.0); 

    for (int i = 0; i < NUM_OF_STEPS; ++i)
    {
        if (t > tFar || accumulatedColor.a >= 0.95) break; 

        vec3 p = ro + rd * t;

        if (p.x > clipX || p.y > clipY || p.z > clipZ) {
            t += STEP_SIZE;
            continue;
        }

        vec3 uvw = (p - BOX_MIN) / (BOX_MAX - BOX_MIN);
        float density = getVolumeDensity(uvw);

        if (density >= isoMin && density <= isoMax) {
            
            vec4 baseColor = getProceduralColor_dvr(density); 

            if (baseColor.a > 0.0) {

                vec3 normal = getVolumeNormal(uvw);
                vec3 lightDir = normalize(light_position - p);
                float diffuse_intensity = max(0.0, dot(normal, lightDir));
                
                float ambient = 0.2; 
                float lighting = max(diffuse_intensity, ambient);

                float currentAlpha = baseColor.a * (1.0 - accumulatedColor.a);
                
                accumulatedColor.rgb += baseColor.rgb * lighting * currentAlpha;
                accumulatedColor.a += currentAlpha;
            }
        }

        t += STEP_SIZE;
    }

    return mix(backgroundColor, accumulatedColor.rgb, accumulatedColor.a);
}


void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(1920.0, 1080.0);
    uv = uv * 2.0 - 1.0;
	uv.x *= (1920.0 / 1080.0); 
	
	vec3 ro = camera_position - vec3(0.0, 0.0, 1.5);
	
    float focalLength = 1.5; 
    
    vec3 rd = normalize(camForward * focalLength + camRight * uv.x + camUp * uv.y);

    light_position = camera_position;

	if (use_dvr) {
		FragColor = vec4(ray_march_dvr(ro, rd), 1.0);
	} 
    else {
		FragColor = vec4(ray_march(ro, rd), 1.0);
	}
}
