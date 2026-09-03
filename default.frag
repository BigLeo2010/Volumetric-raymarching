#version 330 core
out vec4 FragColor;

in vec3 pos;

uniform float time;
uniform vec3 camera_position;
uniform vec3 camForward; 
uniform vec3 camRight;
uniform vec3 camUp;
uniform sampler3D uNoise;

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


float getVolumeDensity(vec3 uvw) {
    float d = texture(uNoise, uvw).r; 
    
    d = d * 0.5 + 0.5; 
    
    float distToCenter = length(uvw - vec3(0.5));
    float sphereMask = smoothstep(0.5, 0.2, distToCenter);
    
    return d * sphereMask;
}

vec4 getIsoColor(float density) {
    float ISO_LEVEL = abs(sin(time * 0.3)); 
    
    if (density >= ISO_LEVEL) {
        return vec4(density, 0, ISO_LEVEL, 1.0); 
    }
    
    return vec4(0.0);
}

//ВРАЩЕНИЕ
vec3 rotateX(vec3 p, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return vec3(p.x, c * p.y - s * p.z, s * p.y + c * p.z);
}

vec3 rotateY(vec3 p, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return vec3(c * p.x + s * p.z, p.y, -s * p.x + c * p.z);
}
vec3 rotateZ(vec3 p, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return vec3(c * p.x - s * p.y, s * p.x + c * p.y, p.z);
}


//ОТРИСОВКА


vec3 ray_march(in vec3 ro, in vec3 rd)
{
	float total_distance_traveled = 0.0;
    float tNear, tFar;

    if (!rayAABBIntersection(ro, rd, BOX_MIN, BOX_MAX, tNear, tFar)) {
        return vec3(35.0/255.0, 35.0/255.0, 35.0/255.0);
    }

    tNear = max(tNear, 0.0);
    
    float pathLength = tFar - tNear;
    const float NUM_OF_STEPS = 128;
    float STEP_SIZE = pathLength / float(NUM_OF_STEPS);
    const float MINIMUM_HIT_DISTANCE = 0.001;
    const float MAXIMUM_TRACE_DISTANCE = 100.0;
    
    float t = tNear;

	for (int i = 0; i < NUM_OF_STEPS; ++i)
	{
		if (t > tFar) break;

        vec3 p = ro + rd * t;

        vec3 uvw = (p - BOX_MIN) / (BOX_MAX - BOX_MIN);

        float density = getVolumeDensity(uvw);

        vec4 color = getIsoColor(density);

        if (color.a > 0.0) {
            return color.rgb;
        }

        t += STEP_SIZE;
	}

	return vec3(35.0/255.0,35.0/255.0,35.0/255.0);
}

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(1920.0, 1080.0);
    uv = uv * 2.0 - 1.0;
	uv.x *= (1920.0 / 1080.0); 
	
	vec3 ro = camera_position;
	float focalLength = 1.5; 
    vec3 rd = normalize(camForward * focalLength + camRight * uv.x + camUp * uv.y);

	FragColor = vec4(ray_march(ro, rd), 1.0);
} 