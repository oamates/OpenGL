//========================================================================================================================================================================================================================
// Shared code for Direct3D
// Platform : WinAPI
// Created  : Oct 14, 2014
// Authors  : Chris Taylor
// Copyright 2014-2016 Oculus VR, LLC All Rights reserved.
//
// Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License"); you may not use the Oculus VR Rift SDK except in compliance with the License,
// which is provided at the time of installation or download, or which otherwise accompanies this software in either electronic or hard copy form.
//
// You may obtain a copy of the License at http://www.oculusvr.com/licenses/LICENSE-3.3
//
// Unless required by applicable law or agreed to in writing, the Oculus VR SDK distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.
//========================================================================================================================================================================================================================

#ifdef _WIN32

#include "Util_Direct3D.h"
#include "Kernel/OVR_Log.h"

namespace OVR {
namespace D3DUtil {

bool VerifyHRESULT(const char* file, int line, HRESULT hr)
{
    if (FAILED(hr))
    {
        LogError("D3D function returned fail HRESULT at %s on line %d : %s", file, line, D3DUtil::GetWindowsErrorString(hr).ToCStr());
        OVR_ASSERT(false);
        return false;
    }
    return true;
}

String GetWindowsErrorString(HRESULT hr)
{
    wchar_t* errorText = nullptr;

    // use system message tables to retrieve error text, allocate buffer on local heap for error text
    // FORMAT_MESSAGE_IGNORE_INSERTS is important -- will fail otherwise, since we're not (and CANNOT) pass insertion parameters
    // 256 is the minimum size for output buffer
    DWORD slen = FormatMessageW(      
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&errorText, 256, nullptr);

    char formatStr[512];
    snprintf(formatStr, sizeof(formatStr), "[Code=%x = %d]", hr, hr);

    String retStr = formatStr;
    if (slen > 0 && errorText)
    {
        retStr += " ";
        retStr += errorText;
        LocalFree(errorText);                                                       // release memory allocated by FormatMessage()
    }
    return retStr;
}

void LogD3DCompileError(HRESULT hr, ID3DBlob* blob)
{
    if (FAILED(hr))
    {
        char* errStr = (char*)blob->GetBufferPointer();
        SIZE_T len = blob->GetBufferSize();

        if (errStr && len > 0)
            LogError("Error compiling shader: %s", errStr);
    }
}

} // namespace D3DUtil
} // namespace OVR

#endif // _WIN32
