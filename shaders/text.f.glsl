uniform sampler2D texture;
void main()
{
    float a = texture2D(texture, gl_TexCoord[0].xy).r;
    gl_FragColor = vec4(gl_Color.rgb, gl_Color.a*a);
}
