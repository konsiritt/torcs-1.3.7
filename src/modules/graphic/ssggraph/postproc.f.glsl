//uniform sampler2D fbo_texture;
//uniform sampler2D fbo_texture_a;
//varying vec2 f_texcoord;

//const vec3 cgv = vec3(0.2125,0.7154,0.0721); //vec3(0.33,0.33,0.33); //

//void main(void) {
//  vec4 irgba = texture2D(fbo_texture,f_texcoord);
//  vec4 irgba_a = texture2D(fbo_texture_a,f_texcoord);
//  gl_FragColor = vec4(irgba[0],irgba[1],irgba_a[2],irgba[3]);
//}
uniform sampler2D fbo_texture;
uniform sampler2D fbo_texture_a;
varying vec2 f_texcoord;

const vec3 cgv = vec3(0.2125,0.7154,0.0721); //vec3(0.33,0.33,0.33); //

void main(void) {
  vec4 irgba = texture2D(fbo_texture,f_texcoord);
  vec4 irgba_a = texture2D(fbo_texture_a,f_texcoord);
  float luminance = dot(irgba.rgb, cgv);
  float luminance_a = dot(irgba_a.rgb, cgv);
  float luminance_total = luminance-luminance_a;
  if (luminance_total < 0 ) 
  {
     gl_FragColor = vec4(-luminance_total,0,0,1);
  }
  else 
  {
     gl_FragColor = vec4(0,luminance_total,0,1);
  }    
}

