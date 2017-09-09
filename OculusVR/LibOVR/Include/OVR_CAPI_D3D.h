//========================================================================================================================================================================================================================
// D3D specific structures used by the CAPI interface.
// Copyright 2014-2016 Oculus VR, LLC All Rights reserved.
//========================================================================================================================================================================================================================

#ifndef OVR_CAPI_D3D_h
#define OVR_CAPI_D3D_h

#include "OVR_CAPI.h"
#include "OVR_Version.h"


#if defined(_WIN32)
#include <Unknwn.h>
#include <guiddef.h>

#if !defined(OVR_EXPORTING_CAPI)

//========================================================================================================================================================================================================================
// ***** Direct3D Specific
//========================================================================================================================================================================================================================

//========================================================================================================================================================================================================================
// Create Texture Swap Chain suitable for use with Direct3D 11 and 12.
//
//  -session Specifies an ovrSession previously returned by ovr_Create.
//  -d3dPtr Specifies the application's D3D11Device to create resources with or the D3D12CommandQueue which must be the same one the application renders to the eye textures with.
//  -desc Specifies requested texture properties. See notes for more info about texture format.
//  -bindFlags Specifies what ovrTextureBindFlags the application requires for this texture chain.
//  -out_TextureSwapChain[out] Returns the created ovrTextureSwapChain, which will be valid upon a successful return value, else it will be NULL.
//                             This texture chain must be eventually destroyed via ovr_DestroyTextureSwapChain before destroying the session with ovr_Destroy.
//  -return Returns an ovrResult indicating success or failure. In the case of failure, use ovr_GetLastErrorInfo to get more information.
// The texture format provided in desc should be thought of as the format the distortion-compositor will use for the ShaderResourceView when reading the contents of the texture. 
// To that end, it is highly recommended that the application requests texture swapchain formats that are in sRGB-space (e.g. OVR_FORMAT_R8G8B8A8_UNORM_SRGB) as the compositor 
// does sRGB-correct rendering. As such, the compositor relies on the GPU's hardware sampler to do the sRGB-to-linear conversion. If the application still prefers to render to a 
// linear format (e.g. OVR_FORMAT_R8G8B8A8_UNORM) while handling the linear-to-gamma conversion via HLSL code, then the application must still request the corresponding sRGB format 
// and also use the ovrTextureMisc_DX_Typeless flag in the ovrTextureSwapChainDesc's Flag field. This will allow the application to create a RenderTargetView that is the desired 
// linear format while the compositor continues to treat it as sRGB. Failure to do so will cause the compositor to apply unexpected gamma conversions leading to gamma-curve artifacts. 
// The ovrTextureMisc_DX_Typeless flag for depth buffer formats (e.g. OVR_FORMAT_D32_FLOAT) is ignored as they are always converted to be typeless.
//
// See ovr_GetTextureSwapChainLength / ovr_GetTextureSwapChainCurrentIndex / ovr_GetTextureSwapChainDesc / ovr_GetTextureSwapChainBufferDX / ovr_DestroyTextureSwapChain
//========================================================================================================================================================================================================================
OVR_PUBLIC_FUNCTION(ovrResult) ovr_CreateTextureSwapChainDX(ovrSession session, IUnknown* d3dPtr, const ovrTextureSwapChainDesc* desc, ovrTextureSwapChain* out_TextureSwapChain);

//========================================================================================================================================================================================================================
// Get a specific buffer within the chain as any compatible COM interface (similar to QueryInterface)
//  -session Specifies an ovrSession previously returned by ovr_Create.
//  -chain Specifies an ovrTextureSwapChain previously returned by ovr_CreateTextureSwapChainDX
//  -index Specifies the index within the chain to retrieve. Must be between 0 and length (see ovr_GetTextureSwapChainLength), or may pass -1 to get the buffer 
//         at the CurrentIndex location. (Saving a call to GetTextureSwapChainCurrentIndex)
//  -iid Specifies the interface ID of the interface pointer to query the buffer for.
//  -out_Buffer[out] Returns the COM interface pointer retrieved.
//  -return Returns an ovrResult indicating success or failure. In the case of failure, use ovr_GetLastErrorInfo to get more information.
//
// Example code:
//         ovr_GetTextureSwapChainBufferDX(session, chain, 0, IID_ID3D11Texture2D, &d3d11Texture);
//         ovr_GetTextureSwapChainBufferDX(session, chain, 1, IID_PPV_ARGS(&dxgiResource));
//========================================================================================================================================================================================================================
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetTextureSwapChainBufferDX(ovrSession session, ovrTextureSwapChain chain, int index, IID iid, void** out_Buffer);

//========================================================================================================================================================================================================================
// Create Mirror Texture which is auto-refreshed to mirror Rift contents produced by this application.
// A second call to ovr_CreateMirrorTextureDX for a given ovrSession before destroying the first one is not supported and will result in an error return.
//  -session Specifies an ovrSession previously returned by ovr_Create.
//  -d3dPtr Specifies the application's D3D11Device to create resources with or the D3D12CommandQueue which must be the same one the application renders to the textures with.
//  -desc Specifies requested texture properties. See notes for more info about texture format.
//  -out_MirrorTexture[out] Returns the created ovrMirrorTexture, which will be valid upon a successful return value, else it will be NULL.
//                          This texture must be eventually destroyed via ovr_DestroyMirrorTexture before destroying the session with ovr_Destroy.
//  -return Returns an ovrResult indicating success or failure. In the case of failure, use ovr_GetLastErrorInfo to get more information.
// The texture format provided in desc should be thought of as the format the compositor will use for the RenderTargetView when writing into mirror texture. To that end, it is highly 
// recommended that the application requests a mirror texture format that is in sRGB-space (e.g. OVR_FORMAT_R8G8B8A8_UNORM_SRGB) as the compositor does sRGB-correct rendering. 
// If however the application wants to still read the mirror texture as a linear format (e.g. OVR_FORMAT_R8G8B8A8_UNORM) and handle the sRGB-to-linear conversion in HLSL code, then it 
// is recommended the application still requests an sRGB format and also use the ovrTextureMisc_DX_Typeless flag in the ovrMirrorTextureDesc's flags field. This will allow the application 
// to bind a ShaderResourceView that is a linear format while the compositor continues to treat is as sRGB. Failure to do so will cause the compositor to apply unexpected gamma conversions 
// leading to gamma-curve artifacts.
//
// Example code:
//         ovrMirrorTexture     mirrorTexture = nullptr;
//         ovrMirrorTextureDesc mirrorDesc = {};
//         mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
//         mirrorDesc.Width  = mirrorWindowWidth;
//         mirrorDesc.Height = mirrorWindowHeight;
//         ovrResult result = ovr_CreateMirrorTextureDX(session, d3d11Device, &mirrorDesc, &mirrorTexture);
//         [...]
//         // Destroy the texture when done with it.
//         ovr_DestroyMirrorTexture(session, mirrorTexture);
//         mirrorTexture = nullptr;
//
// See ovr_GetMirrorTextureBufferDX / ovr_DestroyMirrorTexture
//========================================================================================================================================================================================================================
OVR_PUBLIC_FUNCTION(ovrResult) ovr_CreateMirrorTextureDX(ovrSession session, IUnknown* d3dPtr, const ovrMirrorTextureDesc* desc, ovrMirrorTexture* out_MirrorTexture);


//========================================================================================================================================================================================================================
// Get a the underlying buffer as any compatible COM interface (similar to QueryInterface)
//  -session Specifies an ovrSession previously returned by ovr_Create.
//  -mirrorTexture Specifies an ovrMirrorTexture previously returned by ovr_CreateMirrorTextureDX
//  -iid Specifies the interface ID of the interface pointer to query the buffer for.
//  -out_Buffer[out] Returns the COM interface pointer retrieved.
//  -return Returns an ovrResult indicating success or failure. In the case of failure, use ovr_GetLastErrorInfo to get more information.
//
// Example code:
//         ID3D11Texture2D* d3d11Texture = nullptr;
//         ovr_GetMirrorTextureBufferDX(session, mirrorTexture, IID_PPV_ARGS(&d3d11Texture));
//         d3d11DeviceContext->CopyResource(d3d11TextureBackBuffer, d3d11Texture);
//         d3d11Texture->Release();
//         dxgiSwapChain->Present(0, 0);
//========================================================================================================================================================================================================================
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetMirrorTextureBufferDX(ovrSession session, ovrMirrorTexture mirrorTexture, IID iid, void** out_Buffer);

#endif // !defined(OVR_EXPORTING_CAPI)
#endif // _WIN32
#endif // OVR_CAPI_D3D_h
