__kernel void clcmain(__read_only image2d_t inputImage,
                      __write_only image2d_t outputImage) {
    int2 coord = (int2)(get_global_id(0), get_global_id(1));

    if (coord.x < get_image_width(inputImage) && coord.y < get_image_height(inputImage)) {
        const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
        float4 pixel = read_imagef(inputImage, sampler, coord);

        float gray = 0.299f * pixel.x + 0.587f * pixel.y + 0.114f * pixel.z;

        float4 outputPixel = (float4)(gray, gray, gray, pixel.w);
        write_imagef(outputImage, coord, outputPixel);
    }
}
