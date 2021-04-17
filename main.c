// Microsoft being picky on standard C
#define _CRT_SECURE_NO_WARNINGS

// For UTF-8 strings across WINAPI functions (does it apply to standard libraries too?)
#define UNICODE

#define _USE_MATH_DEFINES //to get math constants
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3dcompiler.h>
// #include <ScrnSave.h>

// A way to link static libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

// The files containing the shaders' source code
#define VERTEX_SHADER_FILENAME L"vertex.hlsl"
#define PIXEL_SHADER_FILENAME L"pixel.hlsl"

// The log files
#define LOG_MAIN_FILENAME "application.log"
#define LOG_D3D_FILENAME "d3d.log"



// Logging stuff: The file is for general purpose, the info queue is used to dump d3d messages to another file
FILE* instanceLog;
ID3D11InfoQueue* debugQueue;

// Direct3D infrastructure
ID3D11Device* graphicsDevice;
ID3D11DeviceContext* graphicsPipeline;
IDXGISwapChain* swapChain;

// The shaders
ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;

// Vertex data layout shared across IA and VS
ID3D11InputLayout* inputLayout; 

// The swap chain buffer that will be used as the render target by the Output Merger
ID3D11RenderTargetView* renderTargetView;

// The depth/stencil used in conjunction with the render target by the Output Merger
ID3D11Texture2D* depthStencilBuffer;
ID3D11DepthStencilState* depthStencilState;
ID3D11DepthStencilView* depthStencilView;

// The rasterizer state
ID3D11RasterizerState* rasterizerState;

// The D3D buffers that will contain the shapes, the indices, and the transforms
ID3D11Buffer* shapeBuffer;
ID3D11Buffer* indexBuffer;
ID3D11Buffer* transformBuffer;


// Axes orientation, observer's point of view:
//
// X: right
// Y: up
// Z: forward
//
// Triangle culling factor is counter-clockwise by default


// The struct that models a 3D RGBA vertex, along with the format description required by the Input Assembly stage
typedef struct {
    float x, y, z;
    float colour[4];
} vertex;

D3D11_INPUT_ELEMENT_DESC vertexInputSpec[2] = {
    {
        .SemanticName = "POSITION",                         // The HLSL semantic associated with this element in a shader input-signature
        .Format = DXGI_FORMAT_R32G32B32_FLOAT,              // The data type of the element data
        .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,  // Offset (in bytes) from the start of the vertex
        .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA       // Identifies the input data class for a single input slot
    },
    {
        .SemanticName = "COLOR",
        .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
        .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
        .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA
    }
};


// The shape vertices themselves, and their indices; buckle up...
vertex shape[] = {
    { 0.4f,  0.4f, -0.4f, {0.3f, 0.3f, 0.3f, 1.0f}},
    { 0.4f, -0.4f, -0.4f, {0.3f, 0.3f, 0.3f, 1.0f}},
    {-0.4f,  0.4f, -0.4f, {0.3f, 0.3f, 0.3f, 1.0f}},
    {-0.4f, -0.4f, -0.4f, {0.3f, 0.3f, 0.3f, 1.0f}},
    { 0.4f,  0.4f,  0.4f, {0.3f, 0.3f, 0.3f, 1.0f}},
    { 0.4f, -0.4f,  0.4f, {0.3f, 0.3f, 0.3f, 1.0f}},
    {-0.4f,  0.4f,  0.4f, {0.3f, 0.3f, 0.3f, 1.0f}},
    {-0.4f, -0.4f,  0.4f, {0.3f, 0.3f, 0.3f, 1.0f}},

    { 0.4f,  0.4f/3, -0.4f/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    { 0.4f, -0.4f/3, -0.4f/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    {-0.4f,  0.4f/3, -0.4f/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    {-0.4f, -0.4f/3, -0.4f/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    { 0.4f,  0.4f/3,  0.4f/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    { 0.4f, -0.4f/3,  0.4f/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    {-0.4f,  0.4f/3,  0.4f/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    {-0.4f, -0.4f/3,  0.4f/3, {0.5f, 0.5f, 0.0f, 1.0f}},

    { 0.4f/3,  0.4f, -0.4f/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    { 0.4f/3, -0.4f, -0.4f/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    {-0.4f/3,  0.4f, -0.4f/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    {-0.4f/3, -0.4f, -0.4f/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    { 0.4f/3,  0.4f,  0.4f/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    { 0.4f/3, -0.4f,  0.4f/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    {-0.4f/3,  0.4f,  0.4f/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    {-0.4f/3, -0.4f,  0.4f/3, {0.5f, 0.0f, 0.5f, 1.0f}},

    { 0.4f/3,  0.4f/3, -0.4f, {0.0f, 0.5f, 0.5f, 1.0f}},
    { 0.4f/3, -0.4f/3, -0.4f, {0.0f, 0.5f, 0.5f, 1.0f}},
    {-0.4f/3,  0.4f/3, -0.4f, {0.0f, 0.5f, 0.5f, 1.0f}},
    {-0.4f/3, -0.4f/3, -0.4f, {0.0f, 0.5f, 0.5f, 1.0f}},
    { 0.4f/3,  0.4f/3,  0.4f, {0.0f, 0.5f, 0.5f, 1.0f}},
    { 0.4f/3, -0.4f/3,  0.4f, {0.0f, 0.5f, 0.5f, 1.0f}},
    {-0.4f/3,  0.4f/3,  0.4f, {0.0f, 0.5f, 0.5f, 1.0f}},
    {-0.4f/3, -0.4f/3,  0.4f, {0.0f, 0.5f, 0.5f, 1.0f}},

    { 0.4f/3,  0.4f/3, -0.4f/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    { 0.4f/3, -0.4f/3, -0.4f/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    {-0.4f/3,  0.4f/3, -0.4f/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    {-0.4f/3, -0.4f/3, -0.4f/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    { 0.4f/3,  0.4f/3,  0.4f/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    { 0.4f/3, -0.4f/3,  0.4f/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    {-0.4f/3,  0.4f/3,  0.4f/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    {-0.4f/3, -0.4f/3,  0.4f/3, {1.0f, 1.0f, 1.0f, 1.0f}},
};

unsigned int indices[] = {

    24, 25, 32,
    25, 33, 32,
    25, 27, 33,
    27, 35, 33,
    27, 26, 35,
    26, 34, 35,
    26, 24, 34,
    24, 32, 34,

    30, 31, 38,
    31, 39, 38,
    31, 29, 39,
    29, 37, 39,
    29, 28, 37,
    28, 36, 37,
    28, 30, 36,
    30, 38, 36,

    12, 13, 36,
    13, 37, 36,
    13, 9, 37,
    9, 33, 37,
    9, 8, 33,
    8, 32, 33,
    8, 12, 32,
    12, 36, 32,

    10, 11, 34,
    11, 35, 34,
    11, 15, 35,
    15, 39, 35,
    15, 14, 39,
    14, 38, 39,
    14, 10, 38,
    10, 34, 38,

    20, 16, 36,
    16, 32, 36,
    16, 18, 32,
    18, 34, 32,
    18, 22, 34,
    22, 38, 34,
    22, 20, 38,
    20, 36, 38,

    17, 21, 33,
    21, 37, 33,
    21, 23, 37,
    23, 39, 37,
    23, 19, 39,
    19, 35, 39,
    19, 17, 35,
    17, 33, 35,





    0, 1, 24,
    0, 24, 26,
    2, 0, 26, 
    2, 26, 27, 
    3, 2, 27, 
    3, 27, 25, 
    1, 3, 25, 
    1, 25, 24,

    6, 7, 30,
    6, 30, 28,
    4, 6, 28,
    4, 28, 29,
    5, 4, 29,
    5, 29, 31,
    7, 5, 31,
    7, 31, 30,

    4, 5, 12,
    4, 12, 8,
    0, 4, 8,
    0, 8, 9,
    1, 0, 9,
    1, 9, 13,
    5, 1, 13,
    5, 13, 12,

    2, 3, 10,
    2, 10, 14,
    6, 2, 14,
    6, 14, 15,
    7, 6, 15,
    7, 15, 11,
    3, 7, 11,
    3, 11, 10,

    4, 0, 20,
    4, 20, 22,
    6, 4, 22,
    6, 22, 18,
    2, 6, 18,
    2, 18, 16,
    0, 2, 16,
    0, 16, 20,

    1, 5, 17,
    1, 17, 19,
    3, 1, 19,
    3, 19, 23,
    7, 3, 23,
    7, 23, 21,
    5, 7, 21,
    5, 21, 17,

};


// The world transforms struct, and the transforms
typedef struct {
    float orientMatrix[4][4];
    float pointTranslateMatrix[4][4];
    float rotateMatrix[4][4];
    float translateMatrix[4][4];
} transformMatrices;

transformMatrices transforms = {
    .orientMatrix = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    },
    .pointTranslateMatrix = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    },
    .rotateMatrix = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    },
    .translateMatrix = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.3f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    },
};




// The whole process of firing up DirectX
void InitD3D(HWND windowHandle) {

    // This is the value that gets constantly checked for potential problems, almost always used by invocations from graphicsDevice
    HRESULT code;

    // Define multisampling here, as it must be the same between the swap chain buffers (which will be render targets) and the depth/stencil buffer
    DXGI_SAMPLE_DESC msDesc = (DXGI_SAMPLE_DESC) {
        .Count = 4,
        .Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN,
    };

    //---------------------------------------------------------------
    // Build D3D11 Infrastructure

    // The swap chain is the sequence of "screen buffers" (the back buffers, really) that will be cyclically drawn on the screen
    //
    // First, an object describing the characteristics of the swap chain is defined
    //
    // About the (DXGI_MODE_DESC) .BufferDesc substructure:
    // MSDN: If you specify the [DIMENSION] as zero when you call the IDXGIFactory::CreateSwapChain method to create a swap chain, the runtime obtains the [DIMENSION] from the output window and assigns this [DIMENSION] value to the swap-chain description.
    // Also, this structure gets filled after creating the swap chain, thus it can be used to retrieve default properties
    DXGI_SWAP_CHAIN_DESC swapChainDescription = (DXGI_SWAP_CHAIN_DESC) {
        .BufferCount = 1,                           // one is a true back buffer, the other will be the "front buffer": the current image). See NOTES, taken form MSDN.
                                                    // 
                                                    // NOTES: In full-screen mode, there is a dedicated front buffer; in windowed mode, the desktop is the front buffer.
                                                    // If you create a swap chain with one buffer, specifying DXGI_SWAP_EFFECT_SEQUENTIAL does not cause the contents of the single buffer to be swapped with the front buffer.
                                                    // When you call IDXGIFactory::CreateSwapChain to create a full-screen swap chain, you typically include the front buffer in this value.
        .BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM,    // describes the backbuffer display mode: use 32-bit color
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,     // how swap chain is to be used
        .OutputWindow = windowHandle,                       // the application window to be used
        .SampleDesc = msDesc,
        .Windowed = FALSE,                                  // start windowed or fullscreen
        .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,             // Options for handling pixels in a display surface after calling IDXGISwapChain.present(...)
    };

    // Create a device, a device context and a swap chain from the specs defined above
    // NOTE: The description gets copied, thus the copy in this scope can be discarded safely
    D3D_FEATURE_LEVEL implementedFeatureLevel;
    code = D3D11CreateDeviceAndSwapChain(
            NULL,                       // Video adapter to use, NULL for default adapter
            D3D_DRIVER_TYPE_HARDWARE,   // Video adapter rendering type (Hardware / Software / ...)
            NULL,                       // Software rasterizer, mandatory if driver type is software
            D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG, // ORable flags for rendering levels - Debug flag enables the InfoQueue messages
            NULL,                       // An array of feature level requests, sorted by desirability; NULL is equivalent to a version list sorted by most recent (D3D11)
            0,                          // Size of the previous array
            D3D11_SDK_VERSION,          // Yeah... keep it like that
            &swapChainDescription,      // The swap chain spec
            &swapChain,
            &graphicsDevice,
            &implementedFeatureLevel,
            &graphicsPipeline
    );
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating Direct3D infrastructure: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Direct3D infrastructure established: code %lx\n", code);

    fprintf(instanceLog, "Feature level available: %x\n", implementedFeatureLevel);
    swapChain->lpVtbl->GetDesc(swapChain, &swapChainDescription);
    fprintf(instanceLog, "Buffer resolution: %u x %u\n", swapChainDescription.BufferDesc.Width, swapChainDescription.BufferDesc.Height);
    fprintf(instanceLog, "Buffer refresh rate: %u / %u\n", swapChainDescription.BufferDesc.RefreshRate.Numerator, swapChainDescription.BufferDesc.RefreshRate.Denominator);

    // Get the debug log
    code = graphicsDevice->lpVtbl->QueryInterface(graphicsDevice, &IID_ID3D11InfoQueue, (LPVOID) &debugQueue);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed getting debug queue: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Debug queue acquired: code %lx\n", code);




    //---------------------------------------------------------------
    // Back buffer and depth/stencil binding

    // Get the actual buffer from the swap chain; its index is 0 and a 2D texture will be used to contain it
    //
    // Notes: Since we set swapChainDesc.SwapEffect to DXGI_SWAP_EFFECT_DISCARD, we only have access to the first buffer, so we set to 0.
    ID3D11Texture2D *pBackBuffer;
    // WHATEVER-CAST
    code = swapChain->lpVtbl->GetBuffer(swapChain, 0, &IID_ID3D11Texture2D, (LPVOID*) &pBackBuffer);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed getting swapchain backbuffer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Acquired backbuffer: code %lx\n", code);
    // use the back buffer address to create the render target
    // MSDN: Creates a render-target view for accessing resource data
    // DOWNCAST
    code = graphicsDevice->lpVtbl->CreateRenderTargetView(graphicsDevice, (LPVOID) pBackBuffer, NULL, &renderTargetView);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating render target view: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Render target view created: code %lx\n", code);
    pBackBuffer->lpVtbl->Release(pBackBuffer);


    // Construct a depth/stencil buffer to use in conjunction with the back buffer
    //
    // ensure that the dimension sizes are exactly the same as the swap chain's ones
    // The format must be the same across the buffer, and the view that will be constructed on top of it
    UINT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    D3D11_TEXTURE2D_DESC depthStencilDesc = (D3D11_TEXTURE2D_DESC) {
        .Width      = swapChainDescription.BufferDesc.Width,
        .Height     = swapChainDescription.BufferDesc.Height,
        .MipLevels  = 1,
        .ArraySize  = 1,
        .Format     = depthStencilFormat,
        .SampleDesc = msDesc,
        .Usage      = D3D11_USAGE_DEFAULT,
        .BindFlags  = D3D11_BIND_DEPTH_STENCIL,
    };
    code = graphicsDevice->lpVtbl->CreateTexture2D(graphicsDevice, &depthStencilDesc, NULL, &depthStencilBuffer);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating depth stencil texture: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Depth stencil texture created: code %lx\n", code);
    // Take caution with the ViewDimension parameter, as it must correspond to the underlying buffer's characteristics
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = (D3D11_DEPTH_STENCIL_VIEW_DESC) {
        .Format = depthStencilFormat,
        .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS,
    };
    // DOWNCAST
    code = graphicsDevice->lpVtbl->CreateDepthStencilView(graphicsDevice, (LPVOID) depthStencilBuffer, &descDSV, &depthStencilView);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating DS view: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "DS view created: code %lx\n", code);


    // Finally, Bind the views together to the Output Merger stage
    graphicsPipeline->lpVtbl->OMSetRenderTargets(graphicsPipeline, 1, &renderTargetView, depthStencilView);




    //---------------------------------------------------------------
    // Setting up the viewport

    // Make a square viewport, taking as much space as it can get
    int screenHeight = swapChainDescription.BufferDesc.Height;
    int screenWidth = swapChainDescription.BufferDesc.Width;
    int viewPortSide = 0;
    int viewPortX = 0;
    int viewPortY = 0;

    if (screenWidth > screenHeight) {
        viewPortSide = screenHeight;
        viewPortX = (screenWidth - screenHeight) / 2;
    }
    else {
        viewPortSide = screenWidth;
        viewPortY = (screenHeight - screenWidth) / 2;
    }

    D3D11_VIEWPORT viewport = (D3D11_VIEWPORT) {
        .TopLeftX = viewPortX, // -1, -1
        .TopLeftY = viewPortY,
        .Width = viewPortSide, // 1, 1
        .Height = viewPortSide,
        .MinDepth = 0,
        .MaxDepth = 1
    };

    graphicsPipeline->lpVtbl->RSSetViewports(graphicsPipeline, 1, &viewport);




    //---------------------------------------------------------------
    // Shader bytecode compilation

    // Take the HLSL sources for the vertex and pixel stages, compile them, and set them up in the pipeline
    ID3D10Blob *vsBlob, *psBlob;
    code = D3DCompileFromFile(VERTEX_SHADER_FILENAME, NULL, NULL, "VShader", "vs_5_0", 0, 0, &vsBlob, NULL);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed compiling vertex shader: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Vertex shader compiled: code %lx\n", code);
    code = D3DCompileFromFile(PIXEL_SHADER_FILENAME, NULL, NULL, "PShader", "ps_5_0", 0, 0, &psBlob, NULL);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed compiling pixel shader: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Pixel shader compiled: code %lx\n", code);
    
    code = graphicsDevice->lpVtbl->CreateVertexShader(graphicsDevice, vsBlob->lpVtbl->GetBufferPointer(vsBlob), vsBlob->lpVtbl->GetBufferSize(vsBlob), NULL, &vertexShader);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating vertex shader: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Vertex shader created: code %lx\n", code);
    code = graphicsDevice->lpVtbl->CreatePixelShader(graphicsDevice, psBlob->lpVtbl->GetBufferPointer(psBlob), psBlob->lpVtbl->GetBufferSize(psBlob), NULL, &pixelShader);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating pixel shader: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Pixel shader created: code %lx\n", code);
    
    graphicsPipeline->lpVtbl->VSSetShader(graphicsPipeline, vertexShader, NULL, 0);
    graphicsPipeline->lpVtbl->PSSetShader(graphicsPipeline, pixelShader, NULL, 0);




    //---------------------------------------------------------------
    // Rasterizer controls

    // No triangle culling is desired here, so this description is created with all the defaults taken from MSDN, except for that
    D3D11_RASTERIZER_DESC rasterizerDesc = {
        .FillMode = D3D11_FILL_SOLID,
        .CullMode = D3D11_CULL_NONE,
        .FrontCounterClockwise = FALSE,
        .DepthBias = 0,
        .SlopeScaledDepthBias = 0.0f,
        .DepthBiasClamp = 0.0f,
        .DepthClipEnable = TRUE,
        .ScissorEnable = FALSE,
        .MultisampleEnable = TRUE,
        .AntialiasedLineEnable = TRUE
    };
    code = graphicsDevice->lpVtbl->CreateRasterizerState(graphicsDevice, &rasterizerDesc, &rasterizerState);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed setting rasterizer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Rasterizer set: code %lx\n", code);
    graphicsPipeline->lpVtbl->RSSetState(graphicsPipeline, rasterizerState);




    //---------------------------------------------------------------
    // Depth/stencil controls

    // This is to establish the depth/stencil's behaviour
    D3D11_DEPTH_STENCIL_DESC dsDesc = (D3D11_DEPTH_STENCIL_DESC) {

        // Depth test parameters
        .DepthEnable = TRUE,
        .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
        .DepthFunc = D3D11_COMPARISON_LESS,

        // Stencil test parameters
        .StencilEnable = TRUE,
        .StencilReadMask = 0xFF,
        .StencilWriteMask = 0xFF,

        // Stencil operations if pixel is front-facing
        .FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP,
        .FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR,
        .FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP,
        .FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS,

        // Stencil operations if pixel is back-facing
        .BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP,
        .BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR,
        .BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP,
        .BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS,
    };
    code = graphicsDevice->lpVtbl->CreateDepthStencilState(graphicsDevice, &dsDesc, &depthStencilState);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed setting depth/stencil: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Depth/stencil set: code %lx\n", code);
    graphicsPipeline->lpVtbl->OMSetDepthStencilState(graphicsPipeline, depthStencilState, 1);




    //---------------------------------------------------------------
    // Input specification

    // Instruct the Input Assembly stage on how the vertex data is structured, and what kind of primitive is expected from it to draw
    code = graphicsDevice->lpVtbl->CreateInputLayout(graphicsDevice, vertexInputSpec, 2, vsBlob->lpVtbl->GetBufferPointer(vsBlob), vsBlob->lpVtbl->GetBufferSize(vsBlob), &inputLayout);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating input layout: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Input layout created: code %lx\n", code);
    graphicsPipeline->lpVtbl->IASetInputLayout(graphicsPipeline, inputLayout);

    graphicsPipeline->lpVtbl->IASetPrimitiveTopology(graphicsPipeline, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);




    //---------------------------------------------------------------
    // Buffer creation

    // The common process:
    //
    // 1. define a specification for the buffer
    // 2. (optional?) craft a subresource accordingly
    // 3. create the actual buffer
    // 
    // When all buffers of a category (Vertex, Index (only one can be there), Constant) are constructed, load them collectively on the suitable stage (Input Assembly for vertices and indices, Pixel/Vertex Shader for constants)

    // VERTICES
    D3D11_BUFFER_DESC vBufferSpec = (D3D11_BUFFER_DESC) {
        .ByteWidth      = sizeof shape,             // The byte size of the vertex buffer
        .BindFlags      = D3D11_BIND_VERTEX_BUFFER, // use as a vertex buffer
        .Usage          = D3D11_USAGE_DEFAULT,
    };
    D3D11_SUBRESOURCE_DATA vInitData = (D3D11_SUBRESOURCE_DATA) {
        .pSysMem = shape,
        .SysMemPitch = 0,
        .SysMemSlicePitch = 0,
    };
    code = graphicsDevice->lpVtbl->CreateBuffer(graphicsDevice, &vBufferSpec, &vInitData, &shapeBuffer);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating vertex buffer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Vertex buffer created: code %lx\n", code);
    UINT stride = sizeof (vertex);
    UINT offset = 0;
    graphicsPipeline->lpVtbl->IASetVertexBuffers(graphicsPipeline, 0, 1, &shapeBuffer, &stride, &offset);

    
    // INDICES
    D3D11_BUFFER_DESC iBufferDesc = (D3D11_BUFFER_DESC) {
        .ByteWidth       = sizeof indices,
        .BindFlags       = D3D11_BIND_INDEX_BUFFER,
        .Usage           = D3D11_USAGE_DEFAULT,
    };
    D3D11_SUBRESOURCE_DATA iInitData = (D3D11_SUBRESOURCE_DATA) {
        .pSysMem = indices,
        .SysMemPitch = 0,
        .SysMemSlicePitch = 0,
    };
    code = graphicsDevice->lpVtbl->CreateBuffer(graphicsDevice, &iBufferDesc, &iInitData, &indexBuffer);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating index buffer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Index buffer created: code %lx\n", code);
    graphicsPipeline->lpVtbl->IASetIndexBuffer(graphicsPipeline, indexBuffer, DXGI_FORMAT_R32_UINT, 0);


    // TRANSFORMS
    D3D11_BUFFER_DESC cBufferDesc = (D3D11_BUFFER_DESC) {
        .ByteWidth      = sizeof transforms,
        .BindFlags      = D3D11_BIND_CONSTANT_BUFFER,
        .Usage          = D3D11_USAGE_DYNAMIC,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };
    D3D11_SUBRESOURCE_DATA cInitData = {
        .pSysMem = &transforms,
        .SysMemPitch = 0,
        .SysMemSlicePitch = 0,
    };
    code = graphicsDevice->lpVtbl->CreateBuffer(graphicsDevice, &cBufferDesc, &cInitData, &transformBuffer);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating transform buffer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Transform buffer created: code %lx\n", code);
    graphicsPipeline->lpVtbl->VSSetConstantBuffers(graphicsPipeline, 0, 1, &transformBuffer);



    fprintf(instanceLog, "Init graphics done\n");
}




// this is the function that releases the Direct3D COM objects
void CleanD3D() {

    // Direct3D is actually incapable of closing when in fullscreen mode
    swapChain->lpVtbl->SetFullscreenState(swapChain, FALSE, NULL);

    // close and release all existing COM objects
    shapeBuffer->lpVtbl->Release(shapeBuffer);
    indexBuffer->lpVtbl->Release(indexBuffer);
    transformBuffer->lpVtbl->Release(transformBuffer);
    vertexShader->lpVtbl->Release(vertexShader);
    pixelShader->lpVtbl->Release(pixelShader);
    depthStencilView->lpVtbl->Release(depthStencilView);
    depthStencilBuffer->lpVtbl->Release(depthStencilBuffer);
    renderTargetView->lpVtbl->Release(renderTargetView);
    swapChain->lpVtbl->Release(swapChain);
    graphicsPipeline->lpVtbl->Release(graphicsPipeline);
    graphicsDevice->lpVtbl->Release(graphicsDevice);
}




float angle = 0;
void applyRotation() {
    angle += 0.001;
    if (angle >= M_PI * 2) angle = 0;

    transforms.rotateMatrix[1][1] = cosf(angle);
    transforms.rotateMatrix[1][2] = -sinf(angle);
    transforms.rotateMatrix[2][1] = sinf(angle);
    transforms.rotateMatrix[2][2] = cosf(angle);
}




// this is the function used to render a single frame
void RenderFrame() {

    // animate the shape!
    applyRotation();

    // clear the back buffer to a black background, and reset the depth stencil buffer
    graphicsPipeline->lpVtbl->ClearRenderTargetView(graphicsPipeline, renderTargetView, (float []) {0.0f, 0.0f, 0.0f, 1.0f});
    graphicsPipeline->lpVtbl->ClearDepthStencilView(graphicsPipeline, depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


    // Map, copy, then unmap. Easy.
    D3D11_MAPPED_SUBRESOURCE mappedBuffer;
    // DOWNCAST
    HRESULT code = graphicsPipeline->lpVtbl->Map(graphicsPipeline, (LPVOID) transformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
    memcpy(mappedBuffer.pData, &transforms, sizeof transforms);
    // DOWNCAST
    graphicsPipeline->lpVtbl->Unmap(graphicsPipeline, (LPVOID) transformBuffer, 0);


    // draw the vertex buffer to the back buffer
    graphicsPipeline->lpVtbl->DrawIndexed(graphicsPipeline, sizeof indices / sizeof (unsigned int), 0, 0);

    // switch the back buffer and the front buffer
    // MSDN: Presents a rendered image to the user.
    code = swapChain->lpVtbl->Present(swapChain, 0, 0);
    
}




LRESULT CALLBACK WindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

        // Default process doesn't terminate application on window destruction (different from WM_CLOSE)
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

        // Note: Paint messages can be ignored, as DirectX is doing all the drawing
    case WM_PAINT:
        return 0;

    default:
        return DefWindowProc(windowHandle, uMsg, wParam, lParam);
    }
}




// BUG: Exiting fullscreen hides window!
// Not fixing it, as the main itself is a temporary solution which I will get rid of once I transform this program in a true screen saver
//
// RATIONALE: The window is set to fullscreen at native resolution, and the viewport is set as square. Hence, viewport size is bound by the smallest dimension
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    (void) hPrevInstance;
    (void) lpCmdLine;

    // A makeshift stdout
    instanceLog = fopen(LOG_MAIN_FILENAME, "w");
    setvbuf(instanceLog, NULL, _IONBF, 0);

    // Register the window class. A window class defines icons and stuff; practically application-specific
    const wchar_t* CLASS_NAME = L"Sample Window Class";
    WNDCLASS wc = (WNDCLASS) { 
        .lpfnWndProc   = WindowProc,
        .hInstance     = hInstance,
        .lpszClassName = CLASS_NAME,
    };
    RegisterClass(&wc);

    // Do create a window now
    const wchar_t* windowTitle = L"Learn to Program DirectX";
    HWND windowHandle = CreateWindowEx(
            0,                              // Optional window styles.
            CLASS_NAME,                     // Window class
            windowTitle,                    // Window text
            WS_OVERLAPPEDWINDOW,            // Window style -- windowed
            // WS_EX_TOPMOST | WS_POPUP,    // fullscreen values  -- not working anymore

            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Size and position
            // CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,

            NULL,                           // Parent window    
            NULL,                           // Menu
            hInstance,                      // Instance handle
            NULL                            // Additional application data
    );
    if (windowHandle == NULL) {
        // Welp, can't even have a window
        fprintf(instanceLog, "Failed creating a window: code %lx\n", GetLastError());
        return -1;
    }


    // Initialize the orientation matrix
    transforms.orientMatrix[0][0] = cos(M_PI_4);
    transforms.orientMatrix[0][1] = -sin(M_PI_4);
    transforms.orientMatrix[1][0] = sin(M_PI_4);
    transforms.orientMatrix[1][1] = cos(M_PI_4);


    // Start the engines!
    InitD3D(windowHandle);


    // The "setVisible(true)" part
    fprintf(instanceLog, "Showing window\n");
    ShowWindow(windowHandle, nCmdShow);


    // Run the message loop. Prabably analogous to the EventQueue
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // D3D - Render the graphics
        RenderFrame();
    }
    fprintf(instanceLog, "Out of the loop. Terminating\n");

    // Flush the device's debug log into the file
    FILE* devlog = fopen(LOG_D3D_FILENAME, "w");
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(devlog, "Executed at time: %02d:%02d\n", st.wHour, st.wMinute);

    int a = debugQueue->lpVtbl->GetNumStoredMessagesAllowedByRetrievalFilter(debugQueue);
    for (int i = a - 1; a >= 0; a--) {

        size_t len;
        debugQueue->lpVtbl->GetMessageW(debugQueue, i, NULL, &len);
        D3D11_MESSAGE* debugMessage = malloc(len);
        debugQueue->lpVtbl->GetMessageW(debugQueue, i, debugMessage, &len);
        fprintf(devlog, "%s\n", debugMessage->pDescription);
        free(debugMessage);
    }
    fclose(devlog);

    // All done, close eveything
    CleanD3D();
    fclose(instanceLog);

    return 0;
}
