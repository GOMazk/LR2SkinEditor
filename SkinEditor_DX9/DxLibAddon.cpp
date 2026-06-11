//#include <DxLib.h>
//
//const void* GetGraphIDirect3DTexture9(int GrHandle)
//{
//    IMAGEDATA* Image;
//
//    // ‰Šú‰»”»’è
//    if (DxSysData.DxLib_InitializeFlag == FALSE)
//    {
//        return NULL;
//    }
//
//    // ƒGƒ‰?”»’è
//    if (GRAPHCHK(GrHandle, Image)) //HANDLECHK(           DX_HANDLETYPE_GRAPH, HAND, *( ( HANDLEINFO ** )&GPOINT ) )
//    {
//        return NULL;
//    }
//
//    switch (GRAWIN.Setting.UseGraphicsAPI)
//    {
//#ifndef DX_NON_DIRECT3D11
//    case GRAPHICS_API_DIRECT3D11_WIN32:
//        return NULL;
//#endif // DX_NON_DIRECT3D11
//
//#ifndef DX_NON_DIRECT3D9
//    case GRAPHICS_API_DIRECT3D9_WIN32:
//        return Image->Hard.Draw[0].Tex->PF->D3D9.Texture;
//#endif // DX_NON_DIRECT3D9
//
//    default:
//        return NULL;
//    }
//}