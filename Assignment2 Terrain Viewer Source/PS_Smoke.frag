#version 330

uniform sampler2D smokeTex;
out vec4 oColor;
void main() 
{
   vec4 col = texture(smokeTex, gl_PointCoord); 
   if(col.r > 0.9 && col.g > 0.9 && col.b > 0.9) discard;

   oColor = vec4(col.rgb, 0.5);  //50% transparency for blending

}
	
