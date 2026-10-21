#version 430 core
out vec4 FragColor;

in vec3 pos;

// Входные юниформы для управления камерой, объёмом и визуализацией
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

// Плоскости отсечения (Clamping/Clipping) вдоль соответствующих осей
uniform float clipX;
uniform float clipY;
uniform float clipZ;

// Границы трёхмерного контейнера (Axis-Aligned Bounding Box)
const vec3 BOX_MIN = vec3(-2.0, -2.0, -2.0);
const vec3 BOX_MAX = vec3( 2.0,  2.0,  2.0);

/**
 * Расчёт пересечения луча с ограничивающим параллелепипедом (AABB) по методу Эми Уильямс (Williams et al.).
 * 
 * Математический принцип:
 * Пересечение трёхмерного куба сводится к нахождению пересечений луча с тремя парами параллельных плоскостей (X, Y, Z).
 * Уравнение луча: P(t) = ro + t * rd.
 * Для плоскости X = boxMin.x: boxMin.x = ro.x + t * rd.x => t = (boxMin.x - ro.x) / rd.x.
 * Находим интервалы [tmin, tmax] для каждой оси. Пересечение этих интервалов даёт отрезок внутри куба.
 */
bool rayAABBIntersection(vec3 ro, vec3 rd, vec3 boxMin, vec3 boxMax, out float tNear, out float tFar) {
    // invRd = 1 / rd. Предотвращает деление на ноль добавлением малого смещения 1e-6.
    vec3 invRd = 1.0 / (rd + vec3(1e-6));
    
    // Вычисляем параметры t для ближних и дальних плоскостей по каждой оси
    vec3 t0 = (boxMin - ro) * invRd;
    vec3 t1 = (boxMax - ro) * invRd;
    
    // Если rd.x отрицательный, то t0.x будет больше, чем t1.x. Сортируем поточечно:
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    
    // tNear — это точка входа в куб (наибольшее из всех минимальных t)
    tNear = max(max(tmin.x, tmin.y), tmin.z);
    // tFar — это точка выхода из куба (наименьшее из всех максимальных t)
    tFar = min(min(tmax.x, tmax.y), tmax.z);
    
    // Луч пересекает AABB, если точка входа ближе точки выхода (tNear < tFar) 
    // и сам куб находится впереди по направлению луча (tFar > 0.0)
    return tNear < tFar && tFar > 0.0;
}

/**
 * Псевдослучайная функция (Хэш) на основе синуса для создания шума.
 * Принимает 2D-координаты пикселя, возвращает псевдослучайное число в диапазоне [0.0, 1.0).
 * Используется для Ray Jittering (устранения эффекта "слоистости" / бандинга).
 */
float hash(vec2 p) {
    return fract(52.9829189 * fract(dot(p, vec2(0.0605, 0.00767))));
}

float getSTBNNoise() {
    ivec2 pixelPos = ivec2(gl_FragCoord.xy) % 128;
    
    int layer = int(frameIndex % 64);
    
    return texelFetch(stbnTexture, ivec3(pixelPos, layer), 0).r;
}


/**
 * Выборка плотности из 3D-текстуры.
 * uvw — нормализованные текстурные координаты в диапазоне [0.0, 1.0]^3.
 */
float getVolumeDensity(vec3 uvw) {
    float d = texture(uNoise, uvw).r; 
    return d; 
}

/**
 * Получение вектора нормали к изоповерхности.
 * Вектор нормали предварительно рассчитан и запечён в RGB каналы 3D-текстуры uNormal.
 * Диапазон значений в текстуре [0, 1] отображается обратно в декартов вектор направления [-1, 1].
 */
vec3 getVolumeNormal(vec3 uvw) {
    vec3 encodedNormal = texture(uNormal, uvw).rgb;
    vec3 normal = encodedNormal * 2.0 - 1.0; // Декодирование: n = tex * 2.0 - 1.0
    return normalize(normal);
}

// Позиция источника света (переопределяется в main как позиция камеры)
vec3 light_position = vec3(0.0, 5.0, 5.0);

/**
 * Алгоритм Ray Marching для поиска изоповерхности (Volume Isosurface Rendering).
 */
vec3 ray_march(in vec3 ro, in vec3 rd)
{
    float tNear, tFar;

    // Шаг 1: Проверка на пересечение с объёмом. Если луч мимо — возвращаем цвет фона (серый).
    if (!rayAABBIntersection(ro, rd, BOX_MIN, BOX_MAX, tNear, tFar)) {
        return vec3(35.0/255.0, 35.0/255.0, 35.0/255.0);
    }

    // Если камера внутри объёма, tNear будет отрицательным. Ограничиваем его нулём.
    tNear = max(tNear, 0.0);
    
    // Вычисление длины пути луча внутри куба и фиксированного шага дискретизации
    float pathLength = tFar - tNear;
    const float NUM_OF_STEPS = 128;
    float STEP_SIZE = pathLength / float(NUM_OF_STEPS);
    
    // Ray Jittering: Сдвигаем начальную точку луча на случайную долю шага.
    // Это превращает упорядоченный бандинг (артефакты дискретизации) в высокочастотный белый шум.
    float jitter;

    if (STBNsmooth) jitter = getSTBNNoise() * STEP_SIZE;
    else jitter = hash(gl_FragCoord.xy) * STEP_SIZE;

    float t = tNear + jitter;

    // Цикл прохода вдоль луча
	for (int i = 0; i < NUM_OF_STEPS; ++i)
    {
        if (t > tFar) break; // Вышли за пределы куба

        // Текущая точка в мировом пространстве: P = ro + t * rd
        vec3 p = ro + rd * t;

        // Отсечение по плоскостям (Clipping)
        if (p.x > clipX || p.y > clipY || p.z > clipZ) {
            t += STEP_SIZE;
            continue;
        }

        // Перевод мировых координат [-2.0, 2.0] в текстурные [0.0, 1.0]
        // Формула линейного переноса: uvw = (p - min) / (max - min)
        vec3 uvw = (p - BOX_MIN) / (BOX_MAX - BOX_MIN);

        // Получаем плотность в текущей точке
        float density = getVolumeDensity(uvw);

        // Проверяем, попали ли мы внутрь заданного диапазона изоповерхности
        if (density >= isoMin && density <= isoMax) {
            
            /**
             * Линейная интерполяция шага (Линейный поиск изоповерхности).
             * Так как шаг STEP_SIZE дискретен, точное значение isoMin находится где-то между 
             * предыдущим шагом (prev_density) и текущим (density).
             * 
             * Математика: Нахождение корня уравнения f(t) = isoMin.
             * Отношение приращений: factor = (isoMin - density_prev) / (density_curr - density_prev)
             * Уточнённое t = mix(t_prev, t_curr, factor)
             */
            if (i > 0) {
                float prev_t = t - STEP_SIZE;
                vec3 prev_p = ro + rd * prev_t;
                vec3 prev_uvw = (prev_p - BOX_MIN) / (BOX_MAX - BOX_MIN);
                float prev_density = getVolumeDensity(prev_uvw);
                
                float deltaDensity = density - prev_density;
                if (abs(deltaDensity) > 1e-5) {
                    float factor = (isoMin - prev_density) / deltaDensity;
                    t = mix(prev_t, t, factor); // Уточняем параметр t
                    p = ro + rd * t;            // Пересчитываем точную точку геометрии
                    uvw = (p - BOX_MIN) / (BOX_MAX - BOX_MIN); // И точные текстурные координаты
                }
            }
            
            // Расчёт нормали для освещения
            vec3 normal = getVolumeNormal(uvw);
            
            // Вектор направления от точки поверхности к источнику света
            vec3 direction_to_light = normalize(light_position - p);

            // Диффузное освещение по закону Ламберта: I = max(0, N dot L)
            float diffuse_intensity = max(0.0, dot(normal, direction_to_light));
            
            // Фоновое (эмбиентное) освещение, чтобы затенённые участки не были абсолютно чёрными
            float ambient = 0.2; 
            float lighting = max(diffuse_intensity, ambient);

            // Расчёт базового цвета с помощью линейной интерполяции двух цветов на основе плотности.
            // Примечание: умножение и деление (rgbColorA/2 и rgbColorB*2) смещают контрастность градиента.
            vec4 baseColor = vec4(mix(rgbColorA / 2.0, rgbColorB * 2.0, density), 1.0);
            
            // Финальный цвет = базовый цвет * коэффициент освещённости
            return baseColor.rgb * lighting;
        }

        // Переход к следующему дискретному шагу вдоль луча
        t += STEP_SIZE;
    }

    // Если луч прошёл сквозь куб и не встретил изоповерхность заданной плотности
    return vec3(35.0/255.0, 35.0/255.0, 35.0/255.0);
}

void main()
{
    // Нормализация координат экрана (gl_FragCoord) в диапазон [0.0, 1.0]
	vec2 uv = gl_FragCoord.xy / vec2(1920.0, 1080.0);
    // Перевод координат из [0, 1] в NDC (Normalized Device Coordinates) [-1.0, 1.0]
    uv = uv * 2.0 - 1.0;
    // Коррекция соотношения сторон (Aspect Ratio), чтобы изображение не растягивалось по горизонтали
	uv.x *= (1920.0 / 1080.0); 
	
    // Вычисление позиции луча (Ray Origin) с небольшим смещением по оси Z назад
	vec3 ro = camera_position - vec3(0.0, 0.0, 1.0);
	
    // Фокусное расстояние виртуальной камеры (управляет углом обзора FOV)
    float focalLength = 1.5; 
    
    // Генерация направления луча (Ray Direction) на основе ортонормированного базиса камеры (Forward, Right, Up)
    // Математически: rd = normalize(F * focalLength + R * uv.x + U * uv.y)
    vec3 rd = normalize(camForward * focalLength + camRight * uv.x + camUp * uv.y);

    // Привязка источника света к камере (эффект "фонарика" на лбу наблюдателя)
    light_position = camera_position;

    // Вызов Ray Marching и запись итогового цвета пикселя с альфа-каналом 1.0
	FragColor = vec4(ray_march(ro, rd), 1.0);
}
