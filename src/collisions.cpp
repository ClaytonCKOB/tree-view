#include<collisions.h>
bool hasSphereBulletCollision(glm::vec4 sphere_center, float sphere_radius, glm::vec4 bullet){
    if((bullet.x >= sphere_center.x-sphere_radius && bullet.x<= sphere_center.x + sphere_radius) &&
        (bullet.y >= sphere_center.y-sphere_radius && bullet.y<= sphere_center.y + sphere_radius) &&
        (sphere_center.z + sphere_radius >= bullet.z && sphere_center.z -sphere_radius <= bullet.z )
    ){
      return true;
    }
    return false;
    
}

bool hasSphereSphereCollision(glm::vec4 sphere_center, float sphere_radius, glm::vec4 sphere_2, float sphere_2radius){
    if((abs(sphere_2.x - sphere_center.x) < sphere_radius + sphere_2radius) &&
       (abs(sphere_2.y - sphere_center.y) < sphere_radius + sphere_2radius) &&
       (abs(sphere_2.z - sphere_center.z) < sphere_radius + sphere_2radius)){
        return true;
    }
    return false;
    
}

bool hasPointPlaneCollision(glm::vec4 point, glm::vec4 plane){
    if(point.y <= plane.y){
        return true;
    }
    return false;
}