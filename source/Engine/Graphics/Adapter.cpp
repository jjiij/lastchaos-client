#include "stdh.h"

#include <Engine/Graphics/Adapter.h>
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Base/Translation.h>
#include <Engine/Base/Console.h>

#ifdef ADAPTER_VULKAN_INFO
// ### Vulkan stuff
#include <vulkan/vulkan.h>
#pragma comment(lib, "vulkan-1.lib")

static VkInstance                        gl_VkInstance;
static CStaticArray<const char*>         gl_VkPhysDeviceExtensions;
static CStaticArray<const char*>         gl_VkInstanceExtensions;
static CStaticArray<const char*>         gl_VkLayers;

BOOL VkCheckDeviceExtensions(VkPhysicalDevice physDevice, const CStaticArray<const char*>& requiredExtensions)
{
    uint32_t deviceExtCount;
    vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &deviceExtCount, nullptr);

    CStaticArray<VkExtensionProperties> deviceExts;
    deviceExts.New(deviceExtCount);
    vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &deviceExtCount, &deviceExts[0]);

    //
    CPrintF("Vulkan: Required extensions:\n");
    for (INDEX i = 0; i < requiredExtensions.Count(); i++)
    {
        CPrintF("      %s\n", (const char*)CTString(requiredExtensions[i]));
    }
    CPrintF("Vulkan: Found %d device extensions:\n", deviceExtCount);
    for (uint32_t i = 0; i < deviceExtCount; i++)
    {
        CPrintF("      %s\n", (const char*)deviceExts[i].extensionName);
    }
    //

    for (INDEX i = 0; i < requiredExtensions.Count(); i++)
    {
        BOOL found = FALSE;
        for (uint32_t j = 0; j < deviceExtCount; j++)
        {
            if (CTString(requiredExtensions[i]) == deviceExts[j].extensionName)
            {
                found = TRUE;
                break;
            }
        }

        if (!found)
        {
            return FALSE;
        }
    }
    return TRUE;
}
// ###
#endif

extern BOOL _bDedicatedServer;
extern const D3DDEVTYPE d3dDevType;


// list of all modes avaliable through CDS
static CListHead _lhCDSModes;

class CResolution {
public:
  PIX   re_pixSizeI;
  PIX   re_pixSizeJ;
};

/*
static CResolution _areResolutions[] =
{
  {  320,  240 },
  {  400,  300 },
  {  480,  360 },
  {  512,  384 },
  {  640,  480 },
  {  720,  540 },
  {  720,  576 },
  {  800,  500 },
  {  800,  600 },
  {  960,  720 },
  { 1024,  768 },
  { 1152,  864 },
  { 1280,  800 },
  { 1280,  960 },
  { 1280, 1024 },
  { 1600, 1200 },
  { 1792, 1344 },
  { 1856, 1392 },
  { 1920, 1440 },
  { 2048, 1536 },

  // dualhead modes
  { 1280,  480 },
  { 1600,  600 },
  { 2048,  768 },
  { 2304,  864 },
  { 2560,  960 },
  { 3200, 1200 },

  // triplehead modes
  { 1920,  480 },
  { 2400,  600 },
  { 3072,  768 },
  { 3456,  864 },
  { 3840,  960 },
  { 4800, 1200 },

  // NTSC HDTV widescreen
  {  848,  480 },
  {  856,  480 },
};*/

static CResolution _areResolutions[] =
{  
  {  640,  480 }, 
  {  800,  500 },
  {  800,  600 },  
  { 1024,  768 },
  { 1152,  864 },
  { 1280,  720 },
  { 1280,  800 },
  { 1280,  960 },
  { 1280, 1024 },	// wide screen
  { 1366, 768  },
  { 1440, 900  },
  { 1600, 900  },
  { 1600, 1200 },   
  { 1680, 1050  },	// wide screen
  { 1920, 1080 },
  { 1920, 1200  },	// wide screen	//  [7/2/2010 rumist]
  { 2048, 1152 },
  { 2560, 1080 },
  { 2560, 1440 },
  { 3200, 1800 },
  { 3840, 2160 },
};
// THIS NUMBER MUST NOT BE OVER 25! (otherwise change it in adapter.h)
static const INDEX MAX_RESOLUTIONS = sizeof(_areResolutions)/sizeof(_areResolutions[0]);


// initialize CDS support (enumerate modes at startup)
void CGfxLibrary::InitAPIs(void)
{
  // no need for gfx when dedicated server is on
  if( _bDedicatedServer) return;

  CPrintF("Initialize CDS support (enumerate modes at startup).\n");
  CDisplayAdapter *pda;
  INDEX iResolution;

  // detect current mode and print to console
  memset( &gl_devmode, 0, sizeof(gl_devmode));
  gl_devmode.dmSize = sizeof(gl_devmode);
  LONG lRes = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &gl_devmode);
  CPrintF( TRANS("Current display: '%s' version %d - %dx%dx%d\n\n"), 
           gl_devmode.dmDeviceName, gl_devmode.dmDriverVersion,
           gl_devmode.dmPelsWidth, gl_devmode.dmPelsHeight, gl_devmode.dmBitsPerPel);

  // fill OpenGL adapter info
  gl_gaAPI[GAT_OGL].ga_ctAdapters = 1;
  gl_gaAPI[GAT_OGL].ga_iCurrentAdapter = 0;
  pda = &gl_gaAPI[GAT_OGL].ga_adaAdapter[0];
  pda->da_ulFlags = DAF_USEGDIFUNCTIONS;
  pda->da_strVendor   = TRANS( "unknown");
  pda->da_strRenderer = TRANS( "Default ICD");
  pda->da_strVersion  = "1.1+";

  // detect modes for OpenGL ICD
  pda->da_ctDisplayModes = 0;
  pda->da_iCurrentDisplayMode = -1;

  // enumerate modes thru resolution list
  for( iResolution=0; iResolution<MAX_RESOLUTIONS; iResolution++)
  {
    DEVMODE devmode;
    memset( &devmode, 0, sizeof(devmode));
    CResolution &re = _areResolutions[iResolution];

    // ask windows if they could set the mode
    devmode.dmSize = sizeof(devmode);
    devmode.dmPelsWidth  = re.re_pixSizeI;
    devmode.dmPelsHeight = re.re_pixSizeJ;
    devmode.dmDisplayFlags = CDS_FULLSCREEN;
    devmode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFLAGS;
    LONG lRes = ChangeDisplaySettings( &devmode, CDS_TEST|CDS_FULLSCREEN);
    // skip if not successfull
    if( lRes!=DISP_CHANGE_SUCCESSFUL) continue;

    // make a new display mode
    CDisplayMode &dm = pda->da_admDisplayModes[pda->da_ctDisplayModes];
    dm.dm_pixSizeI = re.re_pixSizeI;
    dm.dm_pixSizeJ = re.re_pixSizeJ;
    dm.dm_ddDepth  = DISPD_DEFAULT;
    pda->da_ctDisplayModes++;
    CPrintF(TRANS("CDS OpenGL ICD present display mode: %dx%d\n"), re.re_pixSizeI, re.re_pixSizeJ);
  }

  // detect presence of 3Dfx standalone OpenGL driver (for Voodoo1/2)
  char *strDummy;
  char  strBuffer[_MAX_PATH+1];
  int iRes = SearchPath( NULL, "3DFXVGL.DLL", NULL, _MAX_PATH, strBuffer, &strDummy);
  // if present
  if(iRes) {
    // set adapter and force some enumeration of voodoo1/2 display modes
    gl_gaAPI[GAT_OGL].ga_ctAdapters++;
    pda = &gl_gaAPI[GAT_OGL].ga_adaAdapter[1];
    pda->da_ctDisplayModes = 4; // voodoos have only 4 display modes
    pda->da_ulFlags = DAF_ONEWINDOW | DAF_FULLSCREENONLY | DAF_16BITONLY;
    pda->da_strVendor   = "3Dfx";
    pda->da_strRenderer = "3Dfx Voodoo2";
    pda->da_strVersion  = "1.1+";
    CDisplayMode *adm = &pda->da_admDisplayModes[0];
	adm[0].dm_pixSizeI =  512;  adm[0].dm_pixSizeJ = 384;  adm[0].dm_ddDepth = DISPD_16BIT;
    adm[1].dm_pixSizeI =  640;  adm[1].dm_pixSizeJ = 480;  adm[1].dm_ddDepth = DISPD_16BIT;
    adm[2].dm_pixSizeI =  800;  adm[2].dm_pixSizeJ = 600;  adm[2].dm_ddDepth = DISPD_16BIT;
    adm[3].dm_pixSizeI = 1024;  adm[3].dm_pixSizeJ = 768;  adm[3].dm_ddDepth = DISPD_16BIT;
  }

#ifdef ADAPTER_VULKAN_INFO
  // VULKAN
#define GAT_VK 2
  VkApplicationInfo AppInfo = {};
  AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  AppInfo.pApplicationName = "Temp VK to get GPU list";
  AppInfo.applicationVersion = 0;
  AppInfo.pEngineName = "Serious Engine 1";
  AppInfo.engineVersion = 0;
  AppInfo.apiVersion = VK_API_VERSION_1_2;
 
  VkInstanceCreateInfo instanceInfo = {};
  instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceInfo.pApplicationInfo = &AppInfo;

  VkResult r;
  VkExtensionProperties *vk_props = NULL;
  uint32_t instance_extension_count;

  r = vkCreateInstance(&instanceInfo, nullptr, &gl_VkInstance);
  if (r != VK_SUCCESS)
  {
    ASSERT(FALSE);
  }
  
  // get all physical devices
  uint32_t ctMaxPhysDevices = 0;
  vkEnumeratePhysicalDevices(gl_VkInstance, &ctMaxPhysDevices, nullptr);

  if (ctMaxPhysDevices == 0)
  {
      CPrintF("Vulkan: GPU wasn't found\n");
  }

  do {
      r = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
      if (r) {
          break;
      }
      if (instance_extension_count == 0) {
          break;
      }
      // VkExtensionProperties
      vk_props = (VkExtensionProperties*)realloc(vk_props, instance_extension_count * sizeof(VkExtensionProperties));
      // vkEnumerateInstanceExtensionProperties
      r = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, vk_props);

  } while (r == VK_INCOMPLETE);

  if (gl_VkLayers.Count() == 0 || gl_VkLayers.Count() < (int32_t)instance_extension_count) {
      gl_VkInstanceExtensions.New(instance_extension_count);
  }
  // Instance Extensions
  CPrintF("\nVulkan: Instance Extensions:\n");
  for (uint32_t i = 0; i < instance_extension_count; i++) {
      VkExtensionProperties* props = &vk_props[i];
      CPrintF("      %-40s", (const char*)props->extensionName);
      gl_VkInstanceExtensions[i] = (const char*)props->extensionName;
      CPrintF(" : extension revision %d\n", props->specVersion);
  }

   uint32_t physDeviceCount = 0;
   r = vkEnumeratePhysicalDevices(gl_VkInstance, &physDeviceCount, NULL);
   //VK_CHECKERROR(r)
   ASSERT(physDeviceCount > 0);

   CStaticArray<VkPhysicalDevice> physDevices;
   physDevices.New(physDeviceCount);

   CPrintF("Vulkan: Try Enumerate Physical Devices...\n");
   r = vkEnumeratePhysicalDevices(gl_VkInstance, &physDeviceCount, &physDevices[0]);
   //VK_CHECKERROR(r)
   ASSERT(physDeviceCount > 0);

   // vulkan devices count
   if (physDeviceCount > 2) {
     physDeviceCount = 2;
   }

   CPrintF("Vulkan: Physical Devices Count: %d\n", physDeviceCount);

   BOOL extensionsSupported = VkCheckDeviceExtensions(physDevices[0], gl_VkPhysDeviceExtensions);

  // TODO: Vulkan: add only suitable devices, i.e. with:
  // * required formats
  // * queue family indices
  // * present modes
  // * required extensions (VK_KHR_SWAPCHAIN_EXTENSION_NAME)

  for (INDEX iAdapter = 0; iAdapter < ctMaxPhysDevices; iAdapter++)
  {
    pda = &gl_gaAPI[GAT_VK].ga_adaAdapter[iAdapter];
    pda->da_ulFlags = NONE;
    pda->da_ctDisplayModes = 0;
    // pda->da_iCurrentDisplayMode = -1;

    // Are they realy needed?
    // * 32-bits rendering modes support
    // * windowed rendering modes support

    // enumerate modes thru resolution list
    for (iResolution = 0; iResolution < MAX_RESOLUTIONS; iResolution++)
    {
      DEVMODE devmode;
      memset(&devmode, 0, sizeof(devmode));
      CResolution& re = _areResolutions[iResolution];

      // ask windows if they could set the mode
      devmode.dmSize = sizeof(devmode);
      devmode.dmPelsWidth = re.re_pixSizeI;
      devmode.dmPelsHeight = re.re_pixSizeJ;
      devmode.dmDisplayFlags = CDS_FULLSCREEN;
      devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFLAGS;
      LONG lRes = ChangeDisplaySettings(&devmode, CDS_TEST | CDS_FULLSCREEN);
      // skip if not successfull
      if (lRes != DISP_CHANGE_SUCCESSFUL) continue;

      // make a new display mode
      CDisplayMode& dm = pda->da_admDisplayModes[pda->da_ctDisplayModes];
      dm.dm_pixSizeI = re.re_pixSizeI;
      dm.dm_pixSizeJ = re.re_pixSizeJ;
      dm.dm_ddDepth = DISPD_DEFAULT;
      pda->da_ctDisplayModes++;
	  CPrintF(TRANS("CDS Vulkan ICD present display mode: %dx%d\n"), re.re_pixSizeI, re.re_pixSizeJ);
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physDevices[iAdapter], &properties);

    switch (properties.vendorID)
    {
    case 0x1002: pda->da_strVendor = "Advanced Micro Devices"; break;
    case 0x1010: pda->da_strVendor = "Imagination Technologies"; break;
    case 0x10DE: pda->da_strVendor = "NVIDIA"; break;
    case 0x5143: pda->da_strVendor = "Qualcomm"; break;
    case 0x8086: pda->da_strVendor = "Intel"; break;
    default: pda->da_strVendor = TRANS("Vulkan"); break;
    }
    pda->da_strRenderer = properties.deviceName;
    pda->da_strVersion.PrintF("%d.%d",
      VK_VERSION_MAJOR(properties.apiVersion),
      VK_VERSION_MINOR(properties.apiVersion));
    //CPrintF(TRANS("\nCDS Vulkan: vendor code: %x\n"), properties.vendorID);
  }

  gl_gaAPI[GAT_VK].ga_ctAdapters = ctMaxPhysDevices;
  gl_gaAPI[GAT_VK].ga_iCurrentAdapter = 0;

  // destroy temporary instance
  vkDestroyInstance(gl_VkInstance, nullptr);
  // END VULKAN
#endif 

  // try to init Direct3D 9
  BOOL bRes = InitDriver_D3D();
  if( !bRes) {
	  CPrintF(TRANS("CDS Failed to init Direct3D driver!\n"));
	  return;
  } // didn't made it?
  
  // determine DX9 adapters and display modes
  const INDEX ctMaxAdapters = gl_pD3D9->GetAdapterCount();
  INDEX &ctAdapters = gl_gaAPI[GAT_D3D].ga_ctAdapters;
  ctAdapters = 0;

  for( INDEX iAdapter=0; iAdapter<ctMaxAdapters; iAdapter++)
  {
    pda = &gl_gaAPI[1].ga_adaAdapter[ctAdapters];
    pda->da_ulFlags = NONE;
    pda->da_ctDisplayModes = 0;
    INDEX ctModes = gl_pD3D9->GetAdapterModeCount(iAdapter, D3DFMT_X8R8G8B8);
    INDEX iMode;
    HRESULT hr;

    // check whether 32-bits rendering modes are supported
    hr = gl_pD3D9->CheckDeviceType( iAdapter, d3dDevType, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, FALSE);
	if (hr != D3D_OK) {
		pda->da_ulFlags |= DAF_16BITONLY;
		CPrintF("CDS Support 16-bit color only\n");
	} 
   
    // check whether windowed rendering modes are supported
    D3DCAPS9 d3dCaps;
    gl_pD3D9->GetDeviceCaps( iAdapter, d3dDevType, &d3dCaps);
    //if( !(d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED)) pda->da_ulFlags |= DAF_FULLSCREENONLY;

	_pGfx->gl_pd3d9Caps = d3dCaps;		// Date : 2006-05-16(¿ÀÈÄ 4:47:51), By eons

	CPrintF("\n");
    // enumerate modes thru resolution list
    for( iResolution=0; iResolution<MAX_RESOLUTIONS; iResolution++)
    {
      CResolution &re = _areResolutions[iResolution];
      for( iMode=0; iMode<ctModes; iMode++) {
        // if resolution matches and display depth is 16 or 32 bit
        D3DDISPLAYMODE d3dDisplayMode;
		gl_pD3D9->EnumAdapterModes(iAdapter, D3DFMT_X8R8G8B8, iMode, &d3dDisplayMode); // ### DX9}
        
		if( d3dDisplayMode.Width==re.re_pixSizeI && d3dDisplayMode.Height==re.re_pixSizeJ
         && (d3dDisplayMode.Format==D3DFMT_A8R8G8B8 || d3dDisplayMode.Format==D3DFMT_X8R8G8B8
         ||  d3dDisplayMode.Format==D3DFMT_A1R5G5B5 || d3dDisplayMode.Format==D3DFMT_X1R5G5B5 
         ||  d3dDisplayMode.Format==D3DFMT_R5G6B5)) {

          hr = gl_pD3D9->CheckDeviceType( iAdapter, d3dDevType, d3dDisplayMode.Format, d3dDisplayMode.Format, FALSE);
          if( hr!=D3D_OK) continue;

          // make a new display mode
          CDisplayMode &dm = pda->da_admDisplayModes[pda->da_ctDisplayModes];
          dm.dm_pixSizeI = re.re_pixSizeI;
          dm.dm_pixSizeJ = re.re_pixSizeJ;
          dm.dm_ddDepth  = DISPD_DEFAULT;
          pda->da_ctDisplayModes++;
          CPrintF(TRANS("CDS Direct3D ICD present display mode: %dx%d\n"), re.re_pixSizeI, re.re_pixSizeJ);
          break;
        }
      }
    }
	CPrintF("\n");

    // get adapter identifier
    ctAdapters++;
    D3DADAPTER_IDENTIFIER9 d3dAdapterIdentifier;
    gl_pD3D9->GetAdapterIdentifier( iAdapter, 0, /*D3DENUM_NO_WHQL_LEVEL*/ &d3dAdapterIdentifier);
    pda->da_strVendor   = "MS DirectX 9"; // ###
    pda->da_strRenderer = d3dAdapterIdentifier.Description;
    pda->da_strVersion.PrintF("%d.%d.%d.%d", d3dAdapterIdentifier.DriverVersion.HighPart >>16,
                                             d3dAdapterIdentifier.DriverVersion.HighPart & 0xFFFF,
                                             d3dAdapterIdentifier.DriverVersion.LowPart >>16,    
                                             d3dAdapterIdentifier.DriverVersion.LowPart & 0xFFFF);
  }
  // shutdown DX8 (we'll start it again if needed)
  D3DRELEASE( gl_pD3D9, TRUE);
  //if( gl_hiDriver!=NONE) FreeLibrary(gl_hiDriver);
  gl_hiDriver = NONE;
}


// get list of all modes avaliable through CDS -- do not modify/free the returned list
CListHead &CDS_GetModes(void)
{
  return _lhCDSModes;
}


// set given display mode
BOOL CDS_SetMode( PIX pixSizeI, PIX pixSizeJ, enum DisplayDepth dd)
{
  // no need for gfx when dedicated server is on
  if( _bDedicatedServer) return FALSE;

  // prepare general mode parameters
  DEVMODE devmode;
  memset(&devmode, 0, sizeof(devmode));
  devmode.dmSize = sizeof(devmode);
  devmode.dmPelsWidth  = pixSizeI;
  devmode.dmPelsHeight = pixSizeJ;
  devmode.dmDisplayFlags = CDS_FULLSCREEN;
  devmode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFLAGS;
  extern INDEX gap_iRefreshRate;
  if( gap_iRefreshRate>0) {
    devmode.dmFields |= DM_DISPLAYFREQUENCY;
    devmode.dmDisplayFrequency = gap_iRefreshRate;
  }
  // determine bits per pixel to try to set
  SLONG slBPP2 = 0;
  switch(dd) {
  case DISPD_16BIT:
    devmode.dmBitsPerPel = 16;
    slBPP2 = 15;
    devmode.dmFields |= DM_BITSPERPEL;
    break;
  case DISPD_32BIT:
    devmode.dmBitsPerPel = 32;
    slBPP2 = 24;
    devmode.dmFields |= DM_BITSPERPEL;
    break;
  case DISPD_DEFAULT:
    NOTHING;
    break;
  default:
    ASSERT(FALSE);
    NOTHING;
  }

  // try to set primary depth
  LONG lRes = ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);

  // if failed
  if( lRes!=DISP_CHANGE_SUCCESSFUL) {
    // try to set secondary depth
    devmode.dmBitsPerPel = slBPP2;
    LONG lRes2 = ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
    // if failed
    if( lRes2!=DISP_CHANGE_SUCCESSFUL) {
      CTString strError;
      switch(lRes) {
      case DISP_CHANGE_SUCCESSFUL:  strError = "DISP_CHANGE_SUCCESSFUL"; break;
      case DISP_CHANGE_RESTART:     strError = "DISP_CHANGE_RESTART"; break;
      case DISP_CHANGE_BADFLAGS:    strError = "DISP_CHANGE_BADFLAGS"; break;
      case DISP_CHANGE_BADPARAM:    strError = "DISP_CHANGE_BADPARAM"; break;
      case DISP_CHANGE_FAILED:      strError = "DISP_CHANGE_FAILED"; break;
      case DISP_CHANGE_BADMODE:     strError = "DISP_CHANGE_BADMODE"; break;
      case DISP_CHANGE_NOTUPDATED:  strError = "DISP_CHANGE_NOTUPDATED"; break;
      default: strError.PrintF("%d", lRes); break;
      }
      CPrintF(TRANS("CDS error: %s\n"), strError);
      return FALSE;
    }
  }
  // report
  CPrintF(TRANS("  CDS: mode set to %dx%dx%d\n"), pixSizeI, pixSizeJ, devmode.dmBitsPerPel);
  return TRUE;
}


// reset windows to mode chosen by user within windows diplay properties
void CDS_ResetMode(void)
{
  // no need for gfx when dedicated server is on
  if( _bDedicatedServer) return;

  LONG lRes = ChangeDisplaySettings( NULL, 0);
  ASSERT(lRes==DISP_CHANGE_SUCCESSFUL);
  CPrintF(TRANS("  CDS: mode reset to original desktop settings\n"));
}