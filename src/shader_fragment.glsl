#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 1
#define PLANE  2
#define NUMBER 3
#define LEAF   4
uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;

in vec4 cor_interpolada_pelo_rasterizador;


// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main(){
    if (object_id == SPHERE || object_id == NUMBER || object_id == PLANE){
        // Obtemos a posição da câmera utilizando a inversa da matriz que define o
        // sistema de coordenadas da câmera.
        vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
        vec4 camera_position = inverse(view) * origin;

        // O fragmento atual é coberto por um ponto que percente à superfície de um
        // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
        // sistema de coordenadas global (World coordinates). Esta posição é obtida
        // através da interpolação, feita pelo rasterizador, da posição de cada
        // vértice.
        vec4 p = position_world;

        // Normal do fragmento atual, interpolada pelo rasterizador a partir das
        // normais de cada vértice.
        vec4 n = normalize(normal);

        // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
        vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

        // Vetor que define o sentido da câmera em relação ao ponto atual.
        vec4 v = normalize(camera_position - p);

        // Coordenadas de textura U e V
        float U = 0.0;
        float V = 0.0;

        float minx = bbox_min.x;
        float maxx = bbox_max.x;

        float miny = bbox_min.y;
        float maxy = bbox_max.y;

        float minz = bbox_min.z;
        float maxz = bbox_max.z;

        vec3 Kd0;

        switch(object_id){
            case SPHERE:
                float rho = 1.0;
                vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
                p = bbox_center + rho * (position_model - bbox_center)/length(position_model - bbox_center);
                p = p - bbox_center;

                float theta = atan(p.x, p.z);
                float phi = asin(p.y/rho);

                U = (theta + M_PI)/(2*M_PI);
                V = (phi + M_PI_2)/M_PI;

                Kd0 = texture(TextureImage0, vec2(U,V)).rgb;
            break;

            case NUMBER:
                minx = bbox_min.x;
                maxx = bbox_max.x;

                miny = bbox_min.y;
                maxy = bbox_max.y;

                minz = bbox_min.z;
                maxz = bbox_max.z;

                U = (position_model.x - minx)/(maxx - minx);
                V = (position_model.z - minz)/(maxz - minz);

                Kd0 = texture(TextureImage1, vec2(U,V)).rgb;
            break;

            case LEAF:
                minx = bbox_min.x;
                maxx = bbox_max.x;

                miny = bbox_min.y;
                maxy = bbox_max.y;

                minz = bbox_min.z;
                maxz = bbox_max.z;

                U = (position_model.x - minx)/(maxx - minx);
                V = (position_model.z - minz)/(maxz - minz);

                Kd0 = texture(TextureImage1, vec2(U,V)).rgb;
            break;

            case PLANE:
                U = texcoords.x;
                V = texcoords.y;
                Kd0 = texture(TextureImage0, vec2(U,V)).rgb;
            break;
        }

        // Equação de Iluminação
        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);

        color.a = 1;

        // Cor final com correção gamma, considerando monitor sRGB.
        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
} 

