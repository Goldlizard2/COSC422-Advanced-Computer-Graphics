#version 330

in float lightFactor;
in vec2 texCoord;
in vec4 texWeights;
in float grassWeight;

out vec4 outColor;


uniform int toggleWireframe;

uniform sampler2D waterTex; 
uniform sampler2D grassTex; 
uniform sampler2D rockTex; 
uniform sampler2D snowTex; 

void main()
{

    vec4 texWater = texture(waterTex, texCoord) * texWeights.x;
    vec4 texGrass = texture(grassTex, texCoord) * texWeights.y;
    vec4 texRock = texture(rockTex, texCoord) * texWeights.z;
    vec4 texSnow = texture(snowTex, texCoord) * texWeights.w;
     
    
    if(toggleWireframe == 0) {
        outColor = lightFactor * (texWater + texGrass + texRock + texSnow);
    } else{
        outColor = vec4(0, 0, 1, 1) * lightFactor;
    }    
}

