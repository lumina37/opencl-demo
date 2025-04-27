#pragma OPENCL EXTENSION cl_khr_fp16 : enable

__kernel void clcmain(__read_only image2d_t inputImage,
                      __write_only image2d_t outputImage) {
    int2 coord = (int2)(get_global_id(0), get_global_id(1));

    if (coord.x < get_image_width(inputImage) && coord.y < get_image_height(inputImage)) {
        const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
        half4 pixel = read_imageh(inputImage, sampler, coord);

        half gray = 0.299 * pixel.x + 0.587 * pixel.y + 0.114 * pixel.z;

        half4 outputPixel = (half4)(gray, gray, gray, pixel.w);
        write_imageh(outputImage, coord, outputPixel);
    }
}
