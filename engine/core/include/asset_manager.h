//manages the loading, unloading, and access of game assets
//assets are paired with a unique ID for easy access. 
//To reference a spritesheet asset use SpriteSheetID_SpriteIndex
//To reference a texture asset use TextureID
//To reference a sound asset use SoundID
//To reference a font asset use FontID
#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

//uses stbi for image loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <filesystem>
#include <iostream>
namespace engine {

enum class AssetType {
    IMAGE,
    SOUND,
    FONT,
    UNKNOWN
};

struct Asset {
    //TODO: For now this will suffice
    // but eventually we will want to have more metadata about the asset
    // such as a uniqueID that can be leveraged for fast lookup
    AssetType type; // Type of the asset (image, sound, font, etc.)
    std::string name;   //file name of the asset
    stbi_uc* data;   // Pointer to the asset data (could be texture, sound, etc.) null terminated
    bool isLoaded; // Flag to indicate if the asset is loaded
    int width;      // Width of the image (if applicable)
    int height;     // Height of the image (if applicable)
    int channels;   // Number of color channels (if applicable)
};

class AssetManager {
public:
    AssetManager() = default;
    ~AssetManager() = default;

    std::unordered_map<std::string, Asset> image_assets = {
        {"SpriteSheet_00", {AssetType::IMAGE, "TestSpriteSheet", nullptr, false, 0, 0, 0}},
        {"Texture_00", {AssetType::IMAGE, "TestTexture", nullptr, false, 0, 0, 0}}
    };

    const std::string getAssetPath() {
        return "../assets/";
    }

    void loadAllAssets() {
        for (auto& asset : image_assets) {
            Asset& assetInfo = asset.second;
            switch (assetInfo.type) {
                case AssetType::IMAGE:
                {
                    const std::string imagePath = getAssetPath() + assetInfo.name + ".png";
                    assetInfo.data = stbi_load(imagePath.c_str(), &assetInfo.width, &assetInfo.height, &assetInfo.channels, 0);
                    if (!assetInfo.data) {
                        std::cerr << "Failed to load image: " << imagePath << std::endl;
                    } else {
                    assetInfo.isLoaded = true;
                    std::cout << "Loaded image: " << assetInfo.name << " (" << assetInfo.width << "x" << assetInfo.height << ", " << assetInfo.channels << " channels)" << std::endl;
                    }
                    break;
                }              
                case AssetType::SOUND:
                case AssetType::FONT:
                default:
                    std::cerr << "Unknown asset type for asset: " << asset.second.name << std::endl;
                    break;
            }
        }
    }
};

} // namespace engine