#include "graphics.h"

// Needed for PI and trig functions
#define _USE_MATH_DEFINES
#include <math.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

#ifdef DEBUG
#include "debug.h"
#endif

#include "vertex.h"
#include "pixel.h"

#include "dynamenger.h"



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


float startColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};


// Graphics device log
ID3D11InfoQueue* debugQueue;

// Direct3D infrastructure
ID3D11Device* graphicsDevice;
ID3D11DeviceContext* graphicsPipeline;
IDXGISwapChain* swapChain;

// The swap chain buffer that will be used as the render target by the Output Merger
ID3D11RenderTargetView* renderTargetView;

// The depth/stencil used in conjunction with the render target by the Output Merger
ID3D11Texture2D* depthStencilBuffer;
ID3D11DepthStencilState* depthStencilState;
ID3D11DepthStencilView* depthStencilView;

// The rasterizer state
ID3D11RasterizerState* rasterizerState;

// The shaders
ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;

// Vertex data layout shared across IA and VS
ID3D11InputLayout* inputLayout; 

// The D3D buffers that will contain the shapes, the indices, and the transforms
ID3D11Buffer* shapeBuffer;
ID3D11Buffer* indexBuffer;
ID3D11Buffer* transformBuffer;
ID3D11Buffer* colorBuffer;

// The "virtual camera"
D3D11_VIEWPORT viewport;



// Axes orientation, observer's point of view:
//
// X: right
// Y: up
// Z: forward
//
// Triangles that are counter-clockwise are culled by default


// The vertex format description required by the Input Assembly stage; has to accurately match the vertex type
D3D11_INPUT_ELEMENT_DESC vertexInputSpec[2] = {
    {
        .SemanticName = "POSITION",                         // The HLSL semantic associated with this element in a shader input-signature
        .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,              // The data type of the element data
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



// All the description structures; may result useful for quicker changes
DXGI_SWAP_CHAIN_DESC swapChainDescription;
// Define multisampling here, as it must be the same between the swap chain buffers (which will be render targets) and the depth/stencil buffer
DXGI_SAMPLE_DESC msDesc = {
    .Count = 4,
    .Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN,
};
D3D_FEATURE_LEVEL implementedFeatureLevel;
D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
UINT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
D3D11_TEXTURE2D_DESC depthStencilDesc;
D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
D3D11_RASTERIZER_DESC rasterizerDesc;
D3D11_DEPTH_STENCIL_DESC dsDesc;
D3D11_BUFFER_DESC vBufferSpec;
D3D11_SUBRESOURCE_DATA vInitData;
UINT stride;
UINT offset;
D3D11_BUFFER_DESC iBufferDesc;
D3D11_SUBRESOURCE_DATA iInitData;
D3D11_BUFFER_DESC cBufferDesc;
D3D11_SUBRESOURCE_DATA cInitData;





void squareViewport(int screenWidth, int screenHeight) {
    
    // Make a square viewport, taking as much space as it can get
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

    viewport = (D3D11_VIEWPORT) {
        .TopLeftX = (FLOAT) viewPortX, // -1, -1
        .TopLeftY = (FLOAT) viewPortY,
        .Width = (FLOAT) viewPortSide, // 1, 1
        .Height = (FLOAT) viewPortSide,
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };

    graphicsPipeline->lpVtbl->RSSetViewports(graphicsPipeline, 1, &viewport);
}



float shift;
DWORD mode;



// The whole process of firing up DirectX
void InitD3D(HWND windowHandle, DWORD cmode, float rainbowShift) {

    shift = rainbowShift;
    mode = cmode;

    
    // Initialize the orientation matrix
    transforms.orientMatrix[0][0] = cosf((FLOAT) M_PI_4);
    transforms.orientMatrix[0][1] = -sinf((FLOAT) M_PI_4);
    transforms.orientMatrix[1][0] = sinf((FLOAT) M_PI_4);
    transforms.orientMatrix[1][1] = cosf((FLOAT) M_PI_4);

    // Set the screensaver only on the primary screen; these are the metrics to use
    int fsWidth = GetSystemMetrics(SM_CXSCREEN);
    int fsHeight = GetSystemMetrics(SM_CYSCREEN);

    // This is the value that gets constantly checked for potential problems, almost always used by invocations from graphicsDevice
    HRESULT code;



    //---------------------------------------------------------------
    // Build D3D11 Infrastructure

    // The swap chain is the sequence of "screen buffers" (the back buffers, really) that will be cyclically drawn on the screen
    //
    // First, an object describing the characteristics of the swap chain is defined
    //
    // About the (DXGI_MODE_DESC) .BufferDesc substructure:
    // MSDN: If you specify the [DIMENSION] as zero when you call the IDXGIFactory::CreateSwapChain method to create a swap chain, the runtime obtains the [DIMENSION] from the output window and assigns this [DIMENSION] value to the swap-chain description.
    // Also, this structure gets filled after creating the swap chain, thus it can be used to retrieve default properties
    swapChainDescription = (DXGI_SWAP_CHAIN_DESC) {
        .BufferCount = 1,                           // one is a true back buffer, the other will be the "front buffer": the current image). See NOTES, taken form MSDN.
                                                    // 
                                                    // NOTES: In full-screen mode, there is a dedicated front buffer; in windowed mode, the desktop is the front buffer.
                                                    // If you create a swap chain with one buffer, specifying DXGI_SWAP_EFFECT_SEQUENTIAL does not cause the contents of the single buffer to be swapped with the front buffer.
                                                    // When you call IDXGIFactory::CreateSwapChain to create a full-screen swap chain, you typically include the front buffer in this value.
        .BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM,    // describes the backbuffer display mode: use 32-bit color
        .BufferDesc.Width = fsWidth,
        .BufferDesc.Height = fsHeight,
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,     // how swap chain is to be used
        .OutputWindow = windowHandle,
        .SampleDesc = msDesc,
        .Windowed = FALSE,
        .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,             // Options for handling pixels in a display surface after calling IDXGISwapChain.present(...)
    };

    // Create a device, a device context and a swap chain from the specs defined above
    // NOTE: The description gets copied, thus the copy in this scope can be discarded safely
    
    code = D3D11CreateDeviceAndSwapChain(
            NULL,                       // Video adapter to use, NULL for default adapter
            D3D_DRIVER_TYPE_HARDWARE,   // Video adapter rendering type (Hardware / Software / ...)
            NULL,                       // Software rasterizer, mandatory if driver type is software
#ifdef DEBUG
            D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG, // ORable flags for rendering levels - Debug flagenables the InfoQueue messages
#else
            D3D11_CREATE_DEVICE_SINGLETHREADED,
#endif
            NULL,                       // An array of feature level requests, sorted by desirability; NULL is equivalent to a version list sorted by most recent (D3D11)
            0,                          // Size of the previous array
            D3D11_SDK_VERSION,          // Yeah... keep it like that
            &swapChainDescription,      // The swap chain spec
            &swapChain,
            &graphicsDevice,
            &implementedFeatureLevel,
            &graphicsPipeline
    );
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating Direct3D infrastructure: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Direct3D infrastructure established: code %lx\n", code);
#endif

#ifdef DEBUG
    fprintf(instanceLog, "Feature level available: %x\n", implementedFeatureLevel);
#endif
    swapChain->lpVtbl->GetDesc(swapChain, &swapChainDescription);
#ifdef DEBUG
    fprintf(instanceLog, "Buffer resolution: %u x %u\n", swapChainDescription.BufferDesc.Width, swapChainDescription.BufferDesc.Height);
    fprintf(instanceLog, "Buffer refresh rate: %u / %u\n", swapChainDescription.BufferDesc.RefreshRate.Numerator, swapChainDescription.BufferDesc.RefreshRate.Denominator);
#endif

#ifdef DEBUG
    // Get the debug log
    code = graphicsDevice->lpVtbl->QueryInterface(graphicsDevice, &IID_ID3D11InfoQueue, (LPVOID) &debugQueue);
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed getting debug queue: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Debug queue acquired: code %lx\n", code);
#endif



    //---------------------------------------------------------------
    // Back buffer and depth/stencil binding

    // Get the actual buffer from the swap chain; its index is 0 and a 2D texture will be used to contain it
    //
    // Notes: Since we set swapChainDesc.SwapEffect to DXGI_SWAP_EFFECT_DISCARD, we only have access to the first buffer, so we set to 0.
    ID3D11Texture2D *pBackBuffer;
    // WHATEVER-CAST
    code = swapChain->lpVtbl->GetBuffer(swapChain, 0, &IID_ID3D11Texture2D, (LPVOID*) &pBackBuffer);
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed getting swapchain backbuffer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Acquired backbuffer: code %lx\n", code);
#endif

    // use the back buffer address to create the render target
    rtvDesc = (D3D11_RENDER_TARGET_VIEW_DESC) {
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS,
    };
    // MSDN: Creates a render-target view for accessing resource data
    // DOWNCAST
    code = graphicsDevice->lpVtbl->CreateRenderTargetView(graphicsDevice, (LPVOID) pBackBuffer, &rtvDesc, &renderTargetView);
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating render target view: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Render target view created: code %lx\n", code);
#endif
    pBackBuffer->lpVtbl->Release(pBackBuffer);


    // Construct a depth/stencil buffer to use in conjunction with the back buffer
    //
    // ensure that the dimension sizes are exactly the same as the swap chain's ones
    // The format must be the same across the buffer, and the view that will be constructed on top of it
    depthStencilDesc = (D3D11_TEXTURE2D_DESC) {
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
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating depth stencil texture: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Depth stencil texture created: code %lx\n", code);
#endif

    // Take caution with the ViewDimension parameter, as it must correspond to the underlying buffer's characteristics
    dsvDesc = (D3D11_DEPTH_STENCIL_VIEW_DESC) {
        .Format = depthStencilFormat,
        .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS,
    };
    // DOWNCAST
    code = graphicsDevice->lpVtbl->CreateDepthStencilView(graphicsDevice, (LPVOID) depthStencilBuffer, &dsvDesc, &depthStencilView);
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating DS view: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "DS view created: code %lx\n", code);
#endif


    // Finally, Bind the views together to the Output Merger stage
    graphicsPipeline->lpVtbl->OMSetRenderTargets(graphicsPipeline, 1, &renderTargetView, depthStencilView);



    //---------------------------------------------------------------
    // Setting up the viewport

    squareViewport(swapChainDescription.BufferDesc.Width, swapChainDescription.BufferDesc.Height);



    //---------------------------------------------------------------
    // Setting shader objects
    
    code = graphicsDevice->lpVtbl->CreateVertexShader(graphicsDevice, g_VShader, sizeof g_VShader, NULL, &vertexShader);
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating vertex shader: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Vertex shader created: code %lx\n", code);
#endif
    graphicsPipeline->lpVtbl->VSSetShader(graphicsPipeline, vertexShader, NULL, 0);

    code = graphicsDevice->lpVtbl->CreatePixelShader(graphicsDevice, g_PShader, sizeof g_PShader, NULL, &pixelShader);
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating pixel shader: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Pixel shader created: code %lx\n", code);
#endif
    graphicsPipeline->lpVtbl->PSSetShader(graphicsPipeline, pixelShader, NULL, 0);



    //---------------------------------------------------------------
    // Rasterizer controls

    // No triangle culling is desired here, so this description is created with all the defaults taken from MSDN, except for that
    rasterizerDesc = (D3D11_RASTERIZER_DESC) {
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
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed setting rasterizer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Rasterizer set: code %lx\n", code);
#endif
    graphicsPipeline->lpVtbl->RSSetState(graphicsPipeline, rasterizerState);



    //---------------------------------------------------------------
    // Depth/stencil controls

    // This is to establish the depth/stencil's behaviour
    dsDesc = (D3D11_DEPTH_STENCIL_DESC) {

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
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed setting depth/stencil: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Depth/stencil set: code %lx\n", code);
#endif
    graphicsPipeline->lpVtbl->OMSetDepthStencilState(graphicsPipeline, depthStencilState, 1);



    //---------------------------------------------------------------
    // Input specification

    // Instruct the Input Assembly stage on how the vertex data is structured, and what kind of primitive is expected from it to draw
    code = graphicsDevice->lpVtbl->CreateInputLayout(graphicsDevice, vertexInputSpec, 2, g_VShader, sizeof g_VShader, &inputLayout);
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating input layout: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Input layout created: code %lx\n", code);
#endif
    graphicsPipeline->lpVtbl->IASetInputLayout(graphicsPipeline, inputLayout);

    graphicsPipeline->lpVtbl->IASetPrimitiveTopology(graphicsPipeline, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



    //---------------------------------------------------------------
    // Buffers

    // The common process:
    //
    // 1. define a specification for the buffer
    // 2. (optional?) craft a subresource accordingly
    // 3. create the actual buffer
    // 
    // When all buffers of a category (Vertex, Index (only one can be there), Constant) are constructed, load them collectively on the suitable stage (Input Assembly for vertices and indices, Pixel/Vertex Shader for constants)

    // VERTICES
    vBufferSpec = (D3D11_BUFFER_DESC) {
        .ByteWidth      = sponge->vertexCount * sizeof *(sponge->vertices),     // The byte size of the vertex buffer
        .BindFlags      = D3D11_BIND_VERTEX_BUFFER,                                     // use as a vertex buffer
        .Usage          = D3D11_USAGE_DEFAULT,
    };
    vInitData = (D3D11_SUBRESOURCE_DATA) {
        .pSysMem = sponge->vertices,
        .SysMemPitch = 0,
        .SysMemSlicePitch = 0,
    };
    code = graphicsDevice->lpVtbl->CreateBuffer(graphicsDevice, &vBufferSpec, &vInitData, &shapeBuffer);
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating vertex buffer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Vertex buffer created: code %lx\n", code);
#endif
    stride = sizeof *(sponge->vertices);
    offset = 0;
    graphicsPipeline->lpVtbl->IASetVertexBuffers(graphicsPipeline, 0, 1, &shapeBuffer, &stride, &offset);

    
    // INDICES
    iBufferDesc = (D3D11_BUFFER_DESC) {
        .ByteWidth       = sponge->indexCount * sizeof *(sponge->indices),
        .BindFlags       = D3D11_BIND_INDEX_BUFFER,
        .Usage           = D3D11_USAGE_DEFAULT,
    };
    iInitData = (D3D11_SUBRESOURCE_DATA) {
        .pSysMem = sponge->indices,
        .SysMemPitch = 0,
        .SysMemSlicePitch = 0,
    };
    code = graphicsDevice->lpVtbl->CreateBuffer(graphicsDevice, &iBufferDesc, &iInitData, &indexBuffer);
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating index buffer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Index buffer created: code %lx\n", code);
#endif
    graphicsPipeline->lpVtbl->IASetIndexBuffer(graphicsPipeline, indexBuffer, DXGI_FORMAT_R32_UINT, 0);


    // TRANSFORMS
    cBufferDesc = (D3D11_BUFFER_DESC) {
        .ByteWidth      = sizeof transforms,
        .BindFlags      = D3D11_BIND_CONSTANT_BUFFER,
        .Usage          = D3D11_USAGE_DYNAMIC,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };
    cInitData = (D3D11_SUBRESOURCE_DATA) {
        .pSysMem = &transforms,
        .SysMemPitch = 0,
        .SysMemSlicePitch = 0,
    };
    code = graphicsDevice->lpVtbl->CreateBuffer(graphicsDevice, &cBufferDesc, &cInitData, &transformBuffer);
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating transform buffer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Transform buffer created: code %lx\n", code);
#endif
    graphicsPipeline->lpVtbl->VSSetConstantBuffers(graphicsPipeline, 0, 1, &transformBuffer);


    // OVERRIDE FOR COLOUR MODE 0 -- DEFAULT TO WHITE
    if (mode == 0) {
        startColor[0] = 1.0f;
        startColor[1] = 1.0f;
        startColor[2] = 1.0f;
        startColor[3] = 1.0f;
    }

    // COLORS
    cBufferDesc = (D3D11_BUFFER_DESC) {
        .ByteWidth      = sizeof startColor,
        .BindFlags      = D3D11_BIND_CONSTANT_BUFFER,
        .Usage          = D3D11_USAGE_DYNAMIC,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };
    cInitData = (D3D11_SUBRESOURCE_DATA) {
        .pSysMem = &startColor,
        .SysMemPitch = 0,
        .SysMemSlicePitch = 0,
    };
    code = graphicsDevice->lpVtbl->CreateBuffer(graphicsDevice, &cBufferDesc, &cInitData, &colorBuffer);
#ifdef DEBUG
    if (FAILED(code)) {
        fprintf(instanceLog, "Failed creating color buffer: code %lx\n", code);
        ExitProcess(EXIT_FAILURE);
    }
    fprintf(instanceLog, "Color buffer created: code %lx\n", code);
#endif
    graphicsPipeline->lpVtbl->PSSetConstantBuffers(graphicsPipeline, 0, 1, &colorBuffer);



#ifdef DEBUG
    fprintf(instanceLog, "Init graphics done\n");
#endif
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

#ifdef DEBUG
    if (LOG_D3D_FILENAME != NULL) {
        FILE* devLog = fopen(LOG_D3D_FILENAME, "w");
        setvbuf(devLog, NULL, _IONBF, 0);
        SYSTEMTIME st2;
        GetLocalTime(&st2);
        fprintf(devLog, "Device log dump at %04d-%02d-%02d %02d:%02d\n", st2.wYear, st2.wMonth, st2.wDay, st2.wHour, st2.wMinute);

        UINT64 a = debugQueue->lpVtbl->GetNumStoredMessagesAllowedByRetrievalFilter(debugQueue);
        fprintf(devLog, "%llu messages to report\n\n", a);

        for (UINT64 i = 0; i < a; i++) {
            SIZE_T len;
            debugQueue->lpVtbl->GetMessageW(debugQueue, i, NULL, &len);
            D3D11_MESSAGE* debugMessage = malloc(len);
            debugQueue->lpVtbl->GetMessageW(debugQueue, i, debugMessage, &len);
            fprintf(devLog, "%s\n", debugMessage->pDescription);
            free(debugMessage);
            a--;
        }

        fclose(devLog);
    }
#endif
}




float angle = 0;
void applyRotation(void) {
    angle += 0.007f;
    if (angle >= M_PI * 2) angle -= (float) M_PI * 2;

    transforms.rotateMatrix[1][1] = cosf(angle);
    transforms.rotateMatrix[1][2] = -sinf(angle);
    transforms.rotateMatrix[2][1] = sinf(angle);
    transforms.rotateMatrix[2][2] = cosf(angle);
}



// UNUSED
byte src = 0;
void applyColor(void) {

    switch (src) {
        case 0:
        startColor[0] -= shift;
        startColor[1] += shift;
        if (startColor[1] >= 1.0f) src = 1, startColor[0] = 0;
        break;
        case 1:
        startColor[1] -= shift;
        startColor[2] += shift;
        if (startColor[2] >= 1.0f) src = 2, startColor[1] = 0;
        break;
        case 2:
        startColor[2] -= shift;
        startColor[0] += shift;
        if (startColor[0] >= 1.0f) src = 0, startColor[2] = 0;
        break;
        default:
        exit (0);
    }
}

byte src2 = 0;
void applyColor2(void) {

    switch (src2) {
        case 0:
        startColor[1] += shift;
        if (startColor[1] >= 1.0f) src2 = 1, startColor[1] = 1.0f;
        break;
        case 1:
        startColor[0] -= shift;
        if (startColor[0] <= 0.0f) src2 = 2, startColor[0] = 0.0f;
        break;
        case 2:
        startColor[2] += shift;
        if (startColor[2] >= 1.0f) src2 = 3, startColor[2] = 1.0f;
        break;
        case 3:
        startColor[1] -= shift;
        if (startColor[1] <= 0.0f) src2 = 4, startColor[1] = 0.0f;
        break;
        case 4:
        startColor[0] += shift;
        if (startColor[0] >= 1.0f) src2 = 5, startColor[0] = 1.0f;
        break;
        case 5:
        startColor[2] -= shift;
        if (startColor[2] <= 0.0f) src2 = 0, startColor[2] = 0.0f;
        break;
        default:
        exit (0);
    }
}



// this is the function used to render a single frame
static D3D11_MAPPED_SUBRESOURCE mappedBuffer;
void CALLBACK RenderFrame(HWND window, UINT a, UINT_PTR b, DWORD c) {

    // animate the shape!
    applyRotation();

    // shift the color!
    if (mode == 1) applyColor2();

    // clear the back buffer to a black background, and reset the depth stencil buffer
    graphicsPipeline->lpVtbl->ClearRenderTargetView(graphicsPipeline, renderTargetView, (float []) {0.0f, 0.0f, 0.0f, 1.0f});
    graphicsPipeline->lpVtbl->ClearDepthStencilView(graphicsPipeline, depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


    // Map, copy, then unmap. Easy.
    // DOWNCAST
    HRESULT code = graphicsPipeline->lpVtbl->Map(graphicsPipeline, (LPVOID) transformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
    // memcpy(mappedBuffer.pData, &transforms, sizeof transforms);
    MoveMemory(mappedBuffer.pData, &transforms, sizeof transforms);
    // DOWNCAST
    graphicsPipeline->lpVtbl->Unmap(graphicsPipeline, (LPVOID) transformBuffer, 0);
    // DOWNCAST
    HRESULT code2 = graphicsPipeline->lpVtbl->Map(graphicsPipeline, (LPVOID) colorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
    // memcpy(mappedBuffer.pData, &transforms, sizeof transforms);
    MoveMemory(mappedBuffer.pData, &startColor, sizeof startColor);
    // DOWNCAST
    graphicsPipeline->lpVtbl->Unmap(graphicsPipeline, (LPVOID) colorBuffer, 0);


    // draw the vertex buffer to the back buffer
    graphicsPipeline->lpVtbl->DrawIndexed(graphicsPipeline, sponge->indexCount, 0, 0);

    // switch the back buffer and the front buffer
    // MSDN: Presents a rendered image to the user.
    code = swapChain->lpVtbl->Present(swapChain, 0, 0);
    
}




//  NOTE: The code here resemples much like that of InitD3d, omitting debug statemens. May consider pulling out some helper functions
void resizeD3D(int width, int height) {

    // Unbind and release the views from the Output Merger, and all outstanding references to the swap chain's buffers
    graphicsPipeline->lpVtbl->OMSetRenderTargets(graphicsPipeline, 0, NULL, NULL);
    renderTargetView->lpVtbl->Release(renderTargetView);
    depthStencilView->lpVtbl->Release(depthStencilView);
    depthStencilBuffer->lpVtbl->Release(depthStencilBuffer);


    HRESULT code;
    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    swapChain->lpVtbl->ResizeBuffers(swapChain, 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D *pBackBuffer;
    // WHATEVER-CAST
    code = swapChain->lpVtbl->GetBuffer(swapChain, 0, &IID_ID3D11Texture2D, (LPVOID*) &pBackBuffer);

    // use the back buffer address to create the render target
    // MSDN: Creates a render-target view for accessing resource data
    // DOWNCAST
    code = graphicsDevice->lpVtbl->CreateRenderTargetView(graphicsDevice, (LPVOID) pBackBuffer, NULL, &renderTargetView);
    pBackBuffer->lpVtbl->Release(pBackBuffer);


    // Construct a depth/stencil buffer to use in conjunction with the back buffer
    //
    // ensure that the dimension sizes are exactly the same as the swap chain's ones
    // The format must be the same across the buffer, and the view that will be constructed on top of it
    depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc = (D3D11_TEXTURE2D_DESC) {
        .Width      = width,
        .Height     = height,
        .MipLevels  = 1,
        .ArraySize  = 1,
        .Format     = depthStencilFormat,
        .SampleDesc = msDesc,
        .Usage      = D3D11_USAGE_DEFAULT,
        .BindFlags  = D3D11_BIND_DEPTH_STENCIL,
    };
    code = graphicsDevice->lpVtbl->CreateTexture2D(graphicsDevice, &depthStencilDesc, NULL, &depthStencilBuffer);

    // Take caution with the ViewDimension parameter, as it must correspond to the underlying buffer's characteristics
    dsvDesc = (D3D11_DEPTH_STENCIL_VIEW_DESC) {
        .Format = depthStencilFormat,
        .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS,
    };
    // DOWNCAST
    code = graphicsDevice->lpVtbl->CreateDepthStencilView(graphicsDevice, (LPVOID) depthStencilBuffer, &dsvDesc, &depthStencilView);


    // Finally, Bind the views together to the Output Merger stage
    graphicsPipeline->lpVtbl->OMSetRenderTargets(graphicsPipeline, 1, &renderTargetView, depthStencilView);

    // Set up the viewport.
    squareViewport(width, height);
}
