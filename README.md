# 3D Procedurally Generated Terrain
A little project I did to learn about OpenGL and perlin noise. It evolved out of an effort to flesh out a wireframe prototype, and ended up being a months-long journey of learning OpenGL. As a result, LOTS of the code is copy-pasted due to me being impatient and just trying to get a 3D model going. I've linked as many of the papers and sources I've copied and learned from below.

## Sources

 - Perlin noise: https://en.wikipedia.org/wiki/Perlin_noise
 - Fractal noise: https://thebookofshaders.com/13/
 - Learning OpenGL: https://learnopengl.com/
 - Website for retrieving heightmaps: http://terrain.party/
 - Level of detail and nested grids: http://hhoppe.com/proj/geomclipmap/
 - Fast fourier transform: https://www.keithlantz.net/2011/10/ocean-simulation-part-one-using-the-discrete-fourier-transform/
 - Realistic water: https://hydrogen2014imac.files.wordpress.com/2013/02/realisticwater.pdf
 - Code for fresnel water, including a link to glsl code: https://blog.bonzaisoftware.com/tnp/gl-water-tutorial/
 - Realistic ocean lighting (I was unable to decipher the math so this was mostly used for reference): https://hal.archives-ouvertes.fr/inria-00443630

## Screenshots
![Terrain with fresnel water](https://github.com/sarahayu/3Dterrain-OpenGL/blob/master/screenshots/fresnel-waters.png?raw=true)
*Terrain with fresnel water*
![Terrain with FFT water](https://github.com/sarahayu/3Dterrain-OpenGL/blob/master/screenshots/fft-waters.png?raw=true)
*Terrain with FFT water*
![Kualoa Ranch, Oahu](https://github.com/sarahayu/3Dterrain-OpenGL/blob/master/screenshots/kualoa.png?raw=true)
*Kualoa Ranch, Oahu rendered using heightmap*
![Pali Lookout, Oahu](https://github.com/sarahayu/3Dterrain-OpenGL/blob/master/screenshots/pali.png?raw=true)
*Pali Lookout, Oahu rendered using heightmap*
![Diamond Head, Oahu](https://github.com/sarahayu/3Dterrain-OpenGL/blob/master/screenshots/diamond-head.png?raw=true)
*Diamond Head, Oahu rendered using heightmap*