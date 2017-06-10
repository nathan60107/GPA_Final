#version 420 core  
    
//uniform sampler2D tex;  
    
out vec4 color;    
uniform int parameter;
uniform int comparisonBarBorder;
uniform float time;
uniform vec2 magnifierCenter;
layout (binding = 0) uniform sampler2D tex;
layout (binding = 1) uniform sampler2D noise;
    
in VS_OUT
{   
    vec2 texcoord; 
} fs_in; 

float grayScaleFunc(vec2 coor, sampler2D texInput){
	vec3 tex_color = texture(texInput, coor).rgb;
	const vec3 grayscale = vec3(0.212, 0.715, 0.072);
	float Y = dot(tex_color, grayscale);
	return Y;
}

vec4 blur(int half_size, ivec2 xyCoord){
	vec4 color_sum = vec4(0);
	for (int i = -half_size; i <= half_size ; i++)
		for (int j = -half_size; j <= half_size ; j++){
			ivec2 coord = xyCoord/*ivec2(gl_FragCoord.xy)*/ + ivec2(i, j);
			color_sum += texelFetch(tex, coord, 0);
		}	
	int sample_count = (half_size* 2 + 1) * (half_size* 2 + 1);
	return color_sum/ sample_count;
}

#define USE_MIPMAP

#define GOLDEN_ANGLE 2.39996323

#define ITERATIONS 140

mat2 rot = mat2(cos(GOLDEN_ANGLE), sin(GOLDEN_ANGLE), -sin(GOLDEN_ANGLE), cos(GOLDEN_ANGLE));

vec3 Bokeh(sampler2D tex, vec2 uv, float radius, float amount)
{
	vec3 acc = vec3(0.0);
	vec3 div = vec3(0.0);
	vec2 pixel = 1.0 / vec2(600, 600);// / iResolution.xy;
    float r = 1.0;
    vec2 vangle = vec2(0.0,radius); // Start angle
    amount += radius*500.0;
    
	for (int j = 0; j < ITERATIONS; j++)
    {  
        // the approx increase to sqrt(0, 1, 2, 3...)
        r += 1. / r;
	    vangle = rot * vangle;
		#ifdef USE_MIPMAP
		vec3 col = texture(tex, uv + pixel * (r-1.) * vangle, radius).xyz;
        #else
        vec3 col = texture(tex, uv + pixel * (r-1.) * vangle).xyz;
        #endif
        col = col * col * 1.5; // ...contrast it for better highlights - leave this out elsewhere.
		vec3 bokeh = pow(col, vec3(9.0)) * amount+.4;
		acc += col * bokeh;
		div += bokeh;
	}
	return acc / div;
}

vec3 deform(vec2 p)
{
    vec2 q = sin( vec2(1.1,1.2)*time/500 + p );

    float a = atan( q.y, q.x );
    float r = sqrt( dot(q,q) );

    vec2 uv = p*sqrt(1.0+r*r);
    uv += sin( vec2(0.0,0.6) + vec2(1.0,1.1)*time/500);
         
    return texture( tex, uv*0.3).yxx;
}

#define SIGMOID_CONTRAST 12.0

vec4 contrast(vec4 x, float s) {
	return 1.0 / (1.0 + exp(-s * (x - 0.5)));    
}

void main(void)    
{   
	int half_size;
	vec4 color_sum;
	int sample_count;
	vec2 img_size = vec2(1024,768);
	float threshold;
	
	if (gl_FragCoord.x<comparisonBarBorder)
	switch(parameter){
		case 0:
			//Median Blur
			half_size = 2;
			color_sum = vec4(0);
			for (int i = -half_size; i <= half_size ; i++)
				for (int j = -half_size; j <= half_size ; j++){
					ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
					color_sum += texelFetch(tex, coord, 0);
				}	
			sample_count = (half_size* 2 + 1) * (half_size* 2 + 1);
			//color = color_sum/ sample_count;
			
			//Difference of Gaussian (DoG)
			float sigma_e = 2.0f;
			float sigma_r = 2.8f;
			float phi = 3.4f;
			float tau = 0.99f;
			float twoSigmaESquared = 2.0 * sigma_e * sigma_e;
			float twoSigmaRSquared = 2.0 * sigma_r * sigma_r;
			int halfWidth = int(ceil( 2.0 * sigma_r ));
			vec2 sum = vec2(0.0);
			vec2 norm = vec2(0.0);
			for ( int i= -halfWidth; i<= halfWidth; ++i) {
				for ( int j = -halfWidth; j <= halfWidth; ++j ) {
					float d = length(vec2(i,j));
					vec2 kernel= vec2( exp( -d * d / twoSigmaESquared),
					exp( -d * d / twoSigmaRSquared));
					vec4 c= texture(tex,fs_in.texcoord+vec2(i,j)/img_size);
					vec2 L= vec2(0.299 * c.r+ 0.587 * c.g+ 0.114 * c.b);
					norm += 2.0 * kernel;
					sum += kernel * L;
				}
			}
			sum /= norm;
			float H = 100.0 * (sum.x-tau * sum.y);
			float edge =( H > 0.0 )?1.0:2.0 *smoothstep(-2.0, 2.0, phi * H );
			
			//color = vec4(1,1,1,1)-(vec4(1,1,1,1)-vec4(edge,edge,edge,1.0)+(vec4(1,1,1,1)-color_sum/ sample_count));
			
			float nbins = 8.0;
			vec3 tex_color = vec3(floor((vec4(1,1,1,1)-(vec4(1,1,1,1)-vec4(edge,edge,edge,1.0)+(vec4(1,1,1,1)-color_sum/ sample_count))) * nbins) / nbins);
			color = vec4(tex_color, 1.0);
			break;
		case 1:
			//Laplacian Filter
			half_size = 1;
			for (int i = -half_size; i <= half_size ; i++)
				for (int j = -half_size; j <= half_size ; j++){			
					ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
					color_sum -= texelFetch(tex, coord, 0);
				}
			color_sum += texelFetch(tex, ivec2(gl_FragCoord.xy), 0) * 9;
			const vec3 grayscale1 = vec3(0.212, 0.715, 0.072);
			float Y = dot(vec3(color_sum), grayscale1);
			threshold = 0.1;
			color = (Y>threshold)?vec4(1,1,1,1):vec4(0,0,0,1);
			break;
		case 2:
			//Sharpen Filter
			half_size = 1;
			for (int i = -half_size; i <= half_size ; i++)
				for (int j = -half_size; j <= half_size ; j++){
					ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
					color_sum -= texelFetch(tex, coord, 0);
				}
			color_sum += texelFetch(tex, ivec2(gl_FragCoord.xy), 0)*9;
			color = color_sum + texture(tex, fs_in.texcoord);
			break;
		case 3:
			//Pixelization
			int count = 10;
			half_size = count;
			color_sum = vec4(0);			
			float scoorX = gl_FragCoord.x - mod(gl_FragCoord.x, count);
			float scoorY = gl_FragCoord.y - mod(gl_FragCoord.y, count);
			img_size = vec2(1024,768);
			for (int i = -half_size; i <= half_size ; i++)
				for (int j = -half_size; j <= half_size ; j++){
					ivec2 coord = ivec2(scoorX, scoorY) + ivec2(i, j);
					color_sum += texelFetch(tex, coord, 0);
					//color_sum += texture(tex, vec2(coorX, coorY) + vec2(i,j));
				}
			sample_count = (half_size* 2 + 1) * (half_size* 2 + 1);
			color = color_sum/ sample_count;
			break;
		case 4:
			//Fish-Eye Distortion
			float u;
			float v;
			float d;
			float phiAngle;
			float radius = 300;
			float px = gl_FragCoord.x - 300;
			float py = gl_FragCoord.y - 300;
			if (sqrt(pow(px, 2)+pow(py, 2))<=radius){
				float pz = sqrt(pow(radius, 2) - pow(px, 2) - pow(py, 2));
				d = atan(sqrt(pow(px, 2)+pow(py, 2)), pz)/3.14;
				phiAngle = atan(py, px);
				
				u = d*cos(phiAngle)+0.5;
				v = d*sin(phiAngle)+0.5;
				color = texture(tex, vec2(u, v));
			}else{
				color = texture(tex, fs_in.texcoord);
			}
			break;
		case 5:
			//Sine Wave with time (animation)
			float power1 = 10;
			float power2 = 20;
			float sinX= gl_FragCoord.x + power1*sin(gl_FragCoord.y * power2*3.14 + time/100);
			//gl_FragCoord.x += power1*sin(gl_FragCoord.y * power2*3.14 + time/1000);
			color = texelFetch(tex, ivec2(sinX, gl_FragCoord.y), 0);//texture(tex, fs_in.texcoord);
			break;
		case 6:
			//Red-Blue Stereo
			vec4 texture_color_Left = texture(tex, fs_in.texcoord-vec2(0.005,0));	
			vec4 texture_color_Right = texture(tex, fs_in.texcoord+vec2(0.005,0));	
			vec4 texture_color = vec4(texture_color_Left[0]*0.229+texture_color_Left[1]*0.587+texture_color_Left[2]*0.114, texture_color_Right[1], texture_color_Right[2], 1.0);
			color = texture_color;
			break;
		case 7:
			//Bloom Effect
			half_size = 3;
			color_sum = vec4(0);
			vec4 color_sum_double_blur = vec4(0);
			vec4 temp_color_sum = vec4(0);
			sample_count = (half_size* 2 + 1) * (half_size* 2 + 1);
						
			for (int i = -half_size; i <= half_size ; i++){
				for (int j = -half_size; j <= half_size ; j++){
					ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
					temp_color_sum = vec4(0);
					for (int k = -half_size; k <= half_size ; k++){
						for (int l = -half_size; l <= half_size ; l++){
							temp_color_sum += texelFetch(tex, coord + ivec2(k,l), 0);
						}
					}
					color_sum_double_blur += temp_color_sum /=sample_count;
					color_sum += texelFetch(tex, coord, 0);
				}
			}
			color_sum /= sample_count;
			color_sum_double_blur /= sample_count;
			color = (color_sum + color_sum_double_blur + texture(tex, fs_in.texcoord))/3*1.2;
			
			break;
		case 8:
			//Magnifier
			radius = 100;
			px = gl_FragCoord.x;
			py = gl_FragCoord.y;
			if (sqrt(pow(px-magnifierCenter.x, 2)+pow(py-magnifierCenter.y, 2))<=radius){
				color = texelFetch(tex, (ivec2(px, py)+ivec2(magnifierCenter))/2, 0);
			}else{
				color = texture(tex, fs_in.texcoord);
			}
			break;
		case 9:
			//WaterColor
			if (grayScaleFunc(fs_in.texcoord, noise)>0.5){
				color = blur(2, ivec2(gl_FragCoord)+ ivec2(3,0));//texelFetch(tex, ivec2(gl_FragCoord)+ ivec2(1,0), 0);
				//color = texture(tex, vec2(fs_in.texcoord.x+0.01, fs_in.texcoord.y));
			}else if(grayScaleFunc(fs_in.texcoord, noise)<=0.5){
				color = blur(2, ivec2(gl_FragCoord));
				//color = texture(tex, fs_in.texcoord);
			}
			break;
		case 10://https://www.shadertoy.com/view/4d2Xzw
			//Bokeh disc
			vec2 uv = gl_FragCoord.xy / vec2(600, 600);
			//float time = iGlobalTime*.2 + .5;
			float r = .8 - .8*cos((time/1000*.2 + .5) * 6.283);
			float a = 40.0;
			
			//關鍵所在 這行必須拿掉才能正常顯示
			//uv *= vec2(1.0, -1.0);
			
			color = vec4(Bokeh(tex, uv, r, a), 1.0);
			break;
		case 11://https://www.shadertoy.com/view/4sfGRn
			//Radial Blur
			vec2 p = -1.0 + 2.0*gl_FragCoord.xy / vec2(600, 600);

			vec3 col = vec3(0.0);
			vec2 ds = (vec2(0.0,0.0)-p)/64.0;
			float w = 1.0;
			vec2  s = p;
			for( int i=0; i<64; i++ )
			{
				vec3 res = deform( s );
				col += w*smoothstep( 0.0, 1.0, res );
				w *= .99;
				s += ds;
			}
			col = col * 3.5 / 64.0;

			color = vec4( col*0.8, 1.0 );
			break;
		case 12:
			//Log Transformation
			float logC = 3;
			color = vec4(logC * log(vec3(texture(tex, fs_in.texcoord))+vec3(1)), 1);
			break;
		case 13:
			//Power-law(Gamma) Transformation
			float gamma = sin(time/500)+1;//mod(time/500, 2.0)-1.0;
			vec3 texTemp = vec3(texture(tex, fs_in.texcoord));
			vec3 tempvec3 = vec3(pow(texTemp.x, gamma), pow(texTemp.y, gamma), pow(texTemp.z, gamma));
			color = vec4(1 * tempvec3, 1);
			break;
		case 14://https://www.shadertoy.com/view/4slGWn
			//Texture - LOD(failed)
			uv = gl_FragCoord.xy / vec2(600, 600);
			
			float lod = (5.0 + 5.0*sin( time/1000 ))*step( uv.x, 0.5 );
			
			col = texture( tex, vec2(uv.x,1.0-uv.y), lod ).xyz;
			
			color = vec4( col, 1.0 );
			//color = texture( tex, vec2(uv.x,1.0-uv.y), lod );
			break;
		case 15://https://www.shadertoy.com/view/ldcSDB
			//Anisotropic Blur Image Wrap(failed)
			vec2 texel = 1. / vec2(600, 600);
			uv = gl_FragCoord.xy / vec2(600, 600);
			color = contrast(texture(tex, uv), SIGMOID_CONTRAST);
			break;
		/*case 16://https://www.shadertoy.com/view/XdX3D7
			//(failed)
			float perspective = 0.3;
			p = gl_FragCoord.xy / vec2(600, 600);
			float focus = sin(time/500*2.)*.35+.5;
			float blur = 7.*sqrt(abs(p.y - focus));
			
			// perpective version 	
			//vec2 p2 = vec2(p.x-(1.-p.y)*perspective*(p.x*2. - 1.), -p.y);
			
			// simple vesion 
			vec2 p2 = -p;	
			
			color = texture(tex, p2, blur);
			break;*/
		default:
			color = texture(tex, fs_in.texcoord);
			break;
	}
	else{
		color = texture(tex, fs_in.texcoord);
	}
}