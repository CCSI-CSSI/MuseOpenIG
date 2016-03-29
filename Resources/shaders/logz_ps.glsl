#ifdef USE_LOG_DEPTH_BUFFER
uniform float Fcoef;
varying float flogz;

void log_z_ps(float depthoffset)
{
    gl_FragDepth = log2(flogz) * Fcoef * 0.5 + depthoffset;
}
#else
void log_z_ps(float depthoffset)
{
	
}
#endif
