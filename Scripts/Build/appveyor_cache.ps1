$testRoot = Test-Path -Path C:\AppveyorCache\
$testNinja = Test-Path -Path C:\AppveyorCache\Ninja\
$testVulkan = Test-Path -Path C:\AppveyorCache\VulkanSDK\

if($testRoot -and $testNinja -and $testVulkan) {
		echo "Cache already found ..."
		exit 0
}

echo "Preparing Cache Directories ..."
mkdir C:\AppveyorCache\ -ea 0 > $null
mkdir C:\AppveyorCache\Ninja\ -ea 0 > $null
mkdir C:\AppveyorCache\VulkanSDK\ -ea 0 > $null

# Download Ninja
echo "Downloading Ninja ..."
appveyor DownloadFile $env:NINJA_URL -FileName ninja.zip > $null
7z x ninja.zip -oC:\AppveyorCache\Ninja\ > $null

# Download GLFW
# TODO(vasumahesh1): [ARCH]
echo "Downloading GLFW ..."
appveyor DownloadFile $env:GLFW_64_URL -FileName ninja.zip > $null
7z x glfw-3.2.1.bin.WIN64.zip -oC:\AppveyorCache\GLFW\ > $null

# Download and setup Vulkan
echo "Downloading VulkanSDK ..."
cd ..
appveyor DownloadFile $env:VULKAN_URL -FileName Vulkan.exe > $null

echo "Installing VulkanSDK ..."
cmd /c "Vulkan.exe /S && exit" > $null
Copy-Item -Path "C:\VulkanSDK\*" -Destination "C:\AppveyorCache\VulkanSDK\" -Recurse -Force > $null


echo "Dependencies Downloaded."
exit 0