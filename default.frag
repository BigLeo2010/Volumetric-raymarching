#version 330 core
out vec4 FragColor;

in vec3 pos;

uniform float time;
uniform vec3 camera_position;
uniform vec3 camForward; 
uniform vec3 camRight;
uniform vec3 camUp;
uniform sampler3D uNoise;
uniform sampler3D uNormal;
uniform float isoLevel;
uniform vec3 rgbColorA;
uniform vec3 rgbColorB;

//NOISE FUNCTION

const vec3 BOX_MIN = vec3(-2.0, -2.0, -2.0);
const vec3 BOX_MAX = vec3( 2.0,  2.0,  2.0);

bool rayAABBIntersection(vec3 ro, vec3 rd, vec3 boxMin, vec3 boxMax, out float tNear, out float tFar) {
    vec3 invRd = 1.0 / (rd + vec3(1e-6));
    vec3 t0 = (boxMin - ro) * invRd;
    vec3 t1 = (boxMax - ro) * invRd;
    
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    
    tNear = max(max(tmin.x, tmin.y), tmin.z);
    tFar = min(min(tmax.x, tmax.y), tmax.z);
    
    return tNear < tFar && tFar > 0.0;
}

//Ray jittering
float hash(vec2 p) {
    return fract(52.9829189 * fract(dot(p, vec2(0.0605, 0.00767))));
}

float getVolumeDensity(vec3 uvw) {
    float d = texture(uNoise, uvw).r; 
    return d; 
}

//ОСВЕЩЕНИЕ

vec3 getVolumeNormal(vec3 uvw) {
    vec3 encodedNormal = texture(uNormal, uvw).rgb;
    vec3 normal = encodedNormal * 2.0 - 1.0;
    return normalize(normal);
}

//ОТРИСОВКА

vec3 light_position = vec3(0.0, 5.0, 5.0);

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
    
    float jitter = hash(gl_FragCoord.xy) * STEP_SIZE;
    float t = tNear + jitter;

	for (int i = 0; i < NUM_OF_STEPS; ++i)
    {
        if (t > tFar) break;

        vec3 p = ro + rd * t;
        vec3 uvw = (p - BOX_MIN) / (BOX_MAX - BOX_MIN);

        float density = getVolumeDensity(uvw);

        if (density >= isoLevel) {
            //Интерполяция
            if (i > 0) {
                float prev_t = t - STEP_SIZE;
                vec3 prev_p = ro + rd * prev_t;
                vec3 prev_uvw = (prev_p - BOX_MIN) / (BOX_MAX - BOX_MIN);
                float prev_density = getVolumeDensity(prev_uvw);
                
                float deltaDensity = density - prev_density;
                if (abs(deltaDensity) > 1e-5) {
                    float factor = (isoLevel - prev_density) / deltaDensity;
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

            vec4 baseColor = vec4(mix(rgbColorA/2, rgbColorB*2, density), 1.0);
            return baseColor.rgb * lighting;
        }

        t += STEP_SIZE;
    }

    return vec3(35.0/255.0, 35.0/255.0, 35.0/255.0);
}

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(1920.0, 1080.0);
    uv = uv * 2.0 - 1.0;
	uv.x *= (1920.0 / 1080.0); 
	
	vec3 ro = camera_position - vec3(0.0, 0.0, 1.0);
	float focalLength = 1.5; 
    vec3 rd = normalize(camForward * focalLength + camRight * uv.x + camUp * uv.y);

    light_position = camera_position;

	FragColor = vec4(ray_march(ro, rd), 1.0);
} 