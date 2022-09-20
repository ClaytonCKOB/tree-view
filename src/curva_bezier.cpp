#include <curva_bezier.h>

point_t curva_bezier(double t){
    point_t *p1 = (point_t*) malloc(sizeof(point_t));
    point_t *p2 = (point_t*) malloc(sizeof(point_t));
    point_t *p3 = (point_t*) malloc(sizeof(point_t));
    point_t *p4 = (point_t*) malloc(sizeof(point_t));
    point_t *c12 = (point_t*) malloc(sizeof(point_t));
    point_t *c23 = (point_t*) malloc(sizeof(point_t));
    point_t *c = (point_t*) malloc(sizeof(point_t));
    

    p1->x = 0.0f;
    p1->y = 0.0f;
    p1->z = 0.0f;

    p2->x = -1.0f;
    p2->y = 1.0f;
    p2->z = -1.0f;

    p3->x = 0.0f;
    p3->y = 0.0f;
    p3->z = -2.0f;

    p4->x = 1.0f;
    p4->y = 1.0f;
    p4->z = -1.0f;

    if (t > 1.0){
        if (int(t) != aux){
            printf("%d - %d\n", int(t), aux);
            firstCurve = not firstCurve; 
            aux = int(t);
        }
        t = t - int(t);
    }

    if (firstCurve){
        c12->x = p1->x + t*(p2->x - p1->x);  
        c12->y = p1->y + t*(p2->y - p1->y);  
        c12->z = p1->z + t*(p2->z - p1->z);  

        
        c23->x = p2->x + t*(p3->x - p2->x);  
        c23->y = p2->y + t*(p3->y - p2->y);  
        c23->z = p2->z + t*(p3->z - p2->z);  


    }else{
        c12->x = p3->x + t*(p4->x - p3->x);  
        c12->y = p3->y + t*(p4->y - p3->y);  
        c12->z = p3->z + t*(p4->z - p3->z);  

        
        c23->x = p4->x + t*(p1->x - p4->x);  
        c23->y = p4->y + t*(p1->y - p4->y);  
        c23->z = p4->z + t*(p1->z - p4->z);  

    }

    printf("%lf\n", t);

    c->x = c12->x + t*(c23->x - c12->x);  
    c->y = c12->y + t*(c23->y - c12->y);  
    c->z = c12->z + t*(c23->z - c12->z);  

    return c;
}