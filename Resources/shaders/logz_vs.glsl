#ifdef USE_LOG_DEPTH_BUFFER
uniform float Fcoef;
varying float flogz;

vec4 log_z_vs(in vec4 position)
{
	vec4 adjustedClipPosition = position;
	if ( Fcoef > 0.0 )
	{
		adjustedClipPosition.z = (log2(max(1e-6, position.w+1.0))*Fcoef - 1.0) * position.w;
		flogz = 1.0 + position.w;
	}
	return adjustedClipPosition;
}

#else
vec4 log_z_vs(in vec4 position)
{
	return position;
}
#endif