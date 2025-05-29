#pragma OPENCL EXTENSION cl_khr_fp16 : enable

__kernel void clcmain(__read_only image2d_t src, __write_only image2d_t dst) {
    int2 tid = (int2)(get_global_id(0), get_global_id(1));
    int dstWidth = get_image_width(src);
    int dstHeight = get_image_height(src);

    if (tid.x >= dstWidth && tid.y >= dstHeight) {
        return;
    }

    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
    half4 pixel = read_imageh(src, sampler, tid);

    const half4 weight = (half4)(0.299f, 0.587f, 0.114f, 0.0f);
    half gray = dot(weight, pixel);

    half4 outputPixel = (half4)(gray, gray, gray, pixel.w);
    write_imageh(dst, tid, outputPixel);
}
