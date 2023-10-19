#version 330

layout (location = 0) in float posX;
layout (location = 1) in float velocity;
layout (location = 2) in float startTime;
layout (location = 3) in float angle;
layout (location = 4) in float mag;
layout (location = 5) in float omeg;

uniform mat4 mvpMatrix;
uniform float simt;

const float DMAX = 9;   //Maximum distance of a particle.

void main()
{
    vec4 posn = vec4(posX, 0, 0, 1);
    vec4 dir, nor;              //u, n vectors
    float dist, drift, theta;   //d, h values
    dist = velocity * (simt - startTime);
    dist = mod(dist, DMAX); 
    theta = radians(angle);     
    dir = vec4(-sin(theta), cos(theta), 0, 0);
    drift = mag * sin(omeg*dist);
    nor = vec4(-cos(theta), -sin(theta), 0, 0);
    posn =  posn + (dist * dir) + (drift * nor); 

    gl_PointSize = 50;
    gl_Position = mvpMatrix * posn;
}