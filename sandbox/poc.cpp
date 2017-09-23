    //===================================================================================================================================================================================================================
    // texture unit 2 will have depth texture bound with GL_DEPTH_COMPONENT32 internal format
    //===================================================================================================================================================================================================================
    fbo_depth_t geometry_fbo(res_x, res_y, GL_TEXTURE2, GL_DEPTH_COMPONENT32, GL_LINEAR, GL_CLAMP_TO_BORDER);
    const float zero = 0.0f;
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &zero);
    glClearTexImage(geometry_fbo.texture_id, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &zero);


float lower_bound(vec2 pq, float lambda)
{
    const vec4 offset = vec4(1.0 / 1920.0, 1.0 / 1080.0, -1.0 / 1920.0, -1.0 / 1080.0);

    vec4 d0 = vec4(texture(depth_tex, lambda * pq + offset.xy).x,
                   texture(depth_tex, lambda * pq + offset.xw).x,
                   texture(depth_tex, lambda * pq + offset.zy).x,
                   texture(depth_tex, lambda * pq + offset.zw).x);

    vec4 z_aff = z_near / (1.0 - d0);
    z_aff.xy = min(z_aff.xy, z_aff.zw);
    return min(z_aff.x, z_aff.y);
}

