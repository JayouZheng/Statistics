// ...
#pragma once

#include "Math/Box.h"
#include "Math/Vector.h"
#include "Editor/MaterialEditor/Public/MaterialStatsCommon.h"
#include "Engine/TextureCube.h"
#include "Runtime/Renderer/Private/ScenePrivate.h"
#include "Serialization/BufferArchive.h"
#include "Misc/FileHelper.h"
#include "Components/SkeletalMeshComponent.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "LandscapeComponent.h"
#include "Components/ModelComponent.h"
#include "Runtime/RenderCore/Public/RenderUtils.h"

class FBoxContainer
{
public:
	FBoxContainer(FVector InCenter = FVector(0.f), float InUnitSize = 100.f) :
		Center(InCenter), UnitSize(InUnitSize)
	{
		NumTriangles = 0;
		float BoxExtent = InUnitSize * 0.5f;
		BoundingBox = new FBox(FVector(-BoxExtent, -BoxExtent, 0.f), FVector(BoxExtent, BoxExtent, 0.f));
		*BoundingBox = BoundingBox->MoveTo(InCenter);
	}

	~FBoxContainer()
	{
		delete BoundingBox;
	}

	FORCEINLINE FBox GetBoundingBox() const
	{
		if (BoundingBox != nullptr)
		{
			return FBox(*BoundingBox);
		}
		else
		{
			return FBox(EForceInit::ForceInit);
		}
	}

	uint32 NumTriangles;

private:
	FVector Center;
	float UnitSize;
	FBox* BoundingBox;
};

class FExporterHelper
{
public:	

	struct FSceneStaticMeshDataSet
	{
	public:

		FString Name;
		FString OwnerName;
		FString AssetPath;

		uint32 UniqueId;
		uint32 NumVertices;
		uint32 NumTriangles;
		uint32 NumInstances;

		TArray<int32> BoundsIndices;     // First is Mesh...Rest is Instance...
		TArray<int32> TransformsIndices; // First is Mesh...Rest is Instance...
		TArray<int32> UsedMaterialsIndices;
		TArray<int32> UsedMaterialIntancesIndices;

		uint16 NumLODs;
		uint16 CurrentLOD;
	};

	struct FSceneSkeletalMeshDataSet
	{
	public:

		FString Name;
		FString OwnerName;
		FString AssetPath;

		uint32 UniqueId;
		uint32 NumVertices;
		uint32 NumTriangles;
		uint32 NumSections;

		int32 BoundsIndex;     // First is Mesh...Rest is Instance...
		int32 TransformsIndex; // First is Mesh...Rest is Instance...
		TArray<int32> UsedMaterialsIndices;
		TArray<int32> UsedMaterialIntancesIndices;

		uint16 NumLODs;
		uint16 CurrentLOD;
	};

	struct FSceneLandscapeDataSet
	{
	public:

	};

	struct FSceneMaterialDataSet
	{
	public:

		FString Name;
		FString AssetPath;

		// Stats...
		FString TexSamplers;
		FString UserInterpolators;
		FString TexLookups;
		FString VTLookups; // Virtual Texture
		FString ShaderErrors;

		/////////////////////////
		// Material
		FString MaterialDomain;
		FString BlendMode;
		FString DecalBlendMode;
		FString ShadingModel;
		// Translucency...
		FString TranslucencyLightingMode;
		float   TranslucencyDirectionalLightingIntensity;
		// Others...
		uint32 UniqueId;
		uint32 NumInstances;
		uint32 NumRefs;
		TArray<int32> MatInsIndices;
		TArray<int32> UsedTexturesIndices;
		// ShaderInstructionInfo...BPS is Base Pass Shader... 
		int32 BPSCount;
		int32 BPSSurfaceLightmap;
		int32 BPSVolumetricLightmap;
		int32 BPSVertex;
		// Material...
		uint8 TwoSided : 1;
		uint8 bCastRayTracedShadows : 1;
		// Translucency
		uint8 bScreenSpaceReflections : 1;
		uint8 bContactShadows : 1;
		uint8 bUseTranslucencyVertexFog : 1; // Apply Fogging
		uint8 bComputeFogPerPixel : 1;
		uint8 bOutputTranslucentVelocity : 1; // Output Velocity
			// ^ // Advanced...
		uint8 bEnableSeparateTranslucency : 1; // Render After DOF
		uint8 bEnableResponsiveAA : 1;
		uint8 bEnableMobileSeparateTranslucency : 1;
		uint8 bDisableDepthTest : 1;
		uint8 bWriteOnlyAlpha : 1;
		uint8 AllowTranslucentCustomDepthWrites : 1;
		// Mobile
		uint8 bUseFullPrecision : 1;
		uint8 bUseLightmapDirectionality : 1;
		// Forward Shading
		uint8 bUseHQForwardReflections : 1;
		uint8 bUsePlanarForwardReflections : 1;
		/////////////////////////

		void Init(UMaterial* InMaterial, TArray<int32>& InUsedTexturesIndices)
		{
			/////////////////////////
			// Material
			this->MaterialDomain = FExporterHelper::EnumToStringEx<EMaterialDomain>(InMaterial->MaterialDomain);
			this->BlendMode = FExporterHelper::EnumToStringEx<EBlendMode>(InMaterial->BlendMode);
			this->DecalBlendMode = FExporterHelper::EnumToStringEx<EDecalBlendMode>(InMaterial->DecalBlendMode);
			this->ShadingModel = FExporterHelper::EnumToStringEx(InMaterial->GetShadingModels().GetFirstShadingModel());
			this->TwoSided = InMaterial->TwoSided;
			this->bCastRayTracedShadows = InMaterial->bCastRayTracedShadows;
			// Translucency
			this->bScreenSpaceReflections = InMaterial->bScreenSpaceReflections;
			this->bContactShadows = InMaterial->bContactShadows;
			this->TranslucencyLightingMode = FExporterHelper::EnumToStringEx<ETranslucencyLightingMode>(InMaterial->TranslucencyLightingMode);
			this->TranslucencyDirectionalLightingIntensity = InMaterial->TranslucencyDirectionalLightingIntensity;
			this->bUseTranslucencyVertexFog = InMaterial->bUseTranslucencyVertexFog; // Apply Fogging
			this->bComputeFogPerPixel = InMaterial->bComputeFogPerPixel;
			this->bOutputTranslucentVelocity = InMaterial->bOutputTranslucentVelocity; // Output Velocity
				// ^ // Advanced...
			this->bEnableSeparateTranslucency = InMaterial->bEnableSeparateTranslucency; // Render After DOF
			this->bEnableResponsiveAA = InMaterial->bEnableResponsiveAA;
			this->bEnableMobileSeparateTranslucency = InMaterial->bEnableMobileSeparateTranslucency;
			this->bDisableDepthTest = InMaterial->bDisableDepthTest;
			this->bWriteOnlyAlpha = InMaterial->bWriteOnlyAlpha;
			this->AllowTranslucentCustomDepthWrites = InMaterial->AllowTranslucentCustomDepthWrites;
			// Mobile
			this->bUseFullPrecision = InMaterial->bUseFullPrecision;
			this->bUseLightmapDirectionality = InMaterial->bUseLightmapDirectionality;
			// Forward Shading
			this->bUseHQForwardReflections = InMaterial->bUseHQForwardReflections;
			this->bUsePlanarForwardReflections = InMaterial->bUsePlanarForwardReflections;
			// Stats
			FExporterHelper::FShaderStatsInfo MatShaderInfo;
			TArray<FMaterialStatsUtils::FShaderInstructionsInfo> ShaderInstructionInfo;
			FMaterialResource* MatRes = InMaterial->GetMaterialResource(GMaxRHIFeatureLevel);
			FExporterHelper::GetMatertialStatsInfo(ShaderInstructionInfo, MatShaderInfo, MatRes);

			this->TexSamplers = MatShaderInfo.SamplersCount.StrDescription;
			this->UserInterpolators = MatShaderInfo.InterpolatorsCount.StrDescriptionLong;
			this->UserInterpolators.RemoveFromStart("User Interpolators: ");
			this->TexLookups = MatShaderInfo.TextureSampleCount.StrDescription;
			this->VTLookups = MatShaderInfo.VirtualTextureLookupCount.StrDescription;
			this->ShaderErrors = MatShaderInfo.StrShaderErrors;
			if (ShaderInstructionInfo.Num() < 4)
				ShaderInstructionInfo.AddZeroed(4 - ShaderInstructionInfo.Num());
			this->BPSCount = ShaderInstructionInfo[0].InstructionCount;
			this->BPSSurfaceLightmap = ShaderInstructionInfo[1].InstructionCount;
			this->BPSVolumetricLightmap = ShaderInstructionInfo[2].InstructionCount;
			this->BPSVertex = ShaderInstructionInfo[3].InstructionCount;

			/////////////////////////
			this->UniqueId = InMaterial->GetUniqueID();
			this->Name = InMaterial->GetName();
			this->AssetPath = InMaterial->GetPathName();
			this->UsedTexturesIndices = InUsedTexturesIndices;

			this->NumInstances = 0;
			this->NumRefs = 1;
			this->MatInsIndices.Empty();
		}

		bool operator==(const FSceneMaterialDataSet& InElement) const
		{
			if (this->UniqueId == InElement.UniqueId)
				return true;
			else return false;
		}
	};

	struct FSceneMaterialInstanceDataSet
	{
	public:

		FString Name;
		FString AssetPath;

		// ParentData...
		FString ParentName;

		uint32 UniqueId;
		uint32 NumRefs;
		int32  ParentIndex;
		TArray<int32> UsedTexturesIndices;

		bool operator==(const FSceneMaterialInstanceDataSet& InElement) const
		{
			if (this->UniqueId == InElement.UniqueId)
				return true;
			else return false;
		}
	};

	struct FSceneTextureDataSet
	{
	public:

		FString Name;
		FString AssetPath;
		FString Type;

		FString CurrentSize;
		FString PixelFormat;

		FString SourceSize;
		FString SourceFormat;

		uint32 UniqueId;
		uint32 NumRefs;
		int32  LODBias;

		float  CurrentKB;
		float  FullyLoadedKB;

		// Special Format Size...
		float  PVRTC2;
		float  PVRTC4;
		float  ASTC_4x4;	// 8.00 bpp
		float  ASTC_6x6;	// 3.56 bpp
		float  ASTC_8x8;	// 2.00 bpp
		float  ASTC_10x10;	// 1.28 bpp
		float  ASTC_12x12;	// 0.89 bpp

		uint16 CurrentSizeX;
		uint16 CurrentSizeY;
		uint16 SourceSizeX;
		uint16 SourceSizeY;

		uint8  NumResidentMips;
		uint8  NumMipsAllowed;
		uint8  CurrentMips;
		uint8  CompressionNoAlpha : 1;

		bool operator==(const FSceneTextureDataSet& InElement) const
		{
			if (this->UniqueId == InElement.UniqueId)
				return true;
			else return false;
		}
	};

	struct FSceneDataSet
	{
	public:
		TArray<FSceneStaticMeshDataSet>   StaticMeshesTable;
		TArray<FSceneSkeletalMeshDataSet> SkeletalMeshesTable;
		TArray<FSceneLandscapeDataSet>    LandscapesTable;

		TArray<FMatrix> PrimitiveTransforms;

		TArray<FBoxSphereBounds>			  BoundsTable;
		TArray<FSceneMaterialDataSet>		  MaterialsTable;
		TArray<FSceneMaterialInstanceDataSet> MaterialInstancesTable;
		TArray<FSceneTextureDataSet>		  TexturesTable;
	};

	/** structure used to store various statistics extracted from compiled shaders... */
	struct FShaderStatsInfo
	{
		struct FContent
		{
			FString StrDescription;
			FString StrDescriptionLong;
		};

		TMap<ERepresentativeShader, FContent> ShaderInstructionCount;
		FContent SamplersCount;
		FContent InterpolatorsCount;
		FContent TextureSampleCount;
		FContent VirtualTextureLookupCount;
		FString  StrShaderErrors;

		void Reset()
		{
			ShaderInstructionCount.Empty();

			SamplersCount.StrDescription = TEXT("Compiling...");
			SamplersCount.StrDescriptionLong = TEXT("Compiling...");

			InterpolatorsCount.StrDescription = TEXT("Compiling...");
			InterpolatorsCount.StrDescriptionLong = TEXT("Compiling...");

			TextureSampleCount.StrDescription = TEXT("Compiling...");
			TextureSampleCount.StrDescriptionLong = TEXT("Compiling...");

			VirtualTextureLookupCount.StrDescription = TEXT("Compiling...");
			VirtualTextureLookupCount.StrDescriptionLong = TEXT("Compiling...");

			StrShaderErrors.Empty();
		}

		void Empty()
		{
			ShaderInstructionCount.Empty();

			SamplersCount.StrDescription.Empty();
			SamplersCount.StrDescriptionLong.Empty();

			InterpolatorsCount.StrDescription.Empty();
			InterpolatorsCount.StrDescriptionLong.Empty();

			TextureSampleCount.StrDescription.Empty();
			TextureSampleCount.StrDescriptionLong.Empty();

			VirtualTextureLookupCount.StrDescription.Empty();
			VirtualTextureLookupCount.StrDescriptionLong.Empty();

			StrShaderErrors.Empty();
		}

		bool HasErrors()
		{
			return !StrShaderErrors.IsEmpty();
		}
	};

	static inline UWorld* GetWorld()
	{
		UWorld* World = GWorld.GetReference();
		/// UEditorEngine* Editor = GEditor;
		if (GEditor && (GEditor->bIsSimulatingInEditor || GEditor->PlayWorld))
		{
			if (GEditor->PlayWorld)
			{
				World = GEditor->PlayWorld; // Including PIE World...
			}
			else
			{
				// Get PIE World...But Not Used Now...
				FWorldContext* WorldContext = GEngine->GetWorldContextFromPIEInstance(0);
				UWorld* SIEWorld = WorldContext ? WorldContext->World() : nullptr;
				if (SIEWorld)
				{
					World = SIEWorld;
				}
			}
		}

		return World;
	}

	template<typename EnumType>
	static FString EnumToStringEx(EnumType InEnumType)
	{
		static const UEnum* Enum = StaticEnum<EnumType>();
		check(Enum);
		return Enum->GetNameStringByValue(int64(InEnumType));
	}

	/** Helper functions for text output of properties... */
#ifndef CASE_ENUM_TO_TEXT
#define CASE_ENUM_TO_TEXT(txt) case txt: return TEXT(#txt);
#endif

	static FString EnumToString(const EMaterialShadingModel& InMaterialShadingModel)
	{
		switch (InMaterialShadingModel)
		{
			FOREACH_ENUM_EMATERIALSHADINGMODEL(CASE_ENUM_TO_TEXT)
		}
		return TEXT("MSM_DefaultLit");
	}

	static FString EnumToString(const ETextureSourceFormat& InEnumType)
	{
		FString TSF_Invalid("TSF_Invalid");
		FString TSF_G8("TSF_G8");
		FString TSF_BGRA8("TSF_BGRA8");
		FString TSF_BGRE8("TSF_BGRE8");
		FString TSF_RGBA16("TSF_RGBA16");
		FString TSF_RGBA16F("TSF_RGBA16F");

		//@todo: Deprecated!
		FString TSF_RGBA8("TSF_RGBA8");
		//@todo: Deprecated!
		FString TSF_RGBE8("TSF_RGBE8");

		FString TSF_G16("TSF_G16");
		FString TSF_MAX("TSF_MAX");

		switch (InEnumType)
		{
		case ETextureSourceFormat::TSF_Invalid: return TSF_Invalid;
		case ETextureSourceFormat::TSF_G8: return TSF_G8;
		case ETextureSourceFormat::TSF_BGRA8: return TSF_BGRA8;
		case ETextureSourceFormat::TSF_BGRE8: return TSF_BGRE8;
		case ETextureSourceFormat::TSF_RGBA16: return TSF_RGBA16;
		case ETextureSourceFormat::TSF_RGBA16F: return TSF_RGBA16F;
		case ETextureSourceFormat::TSF_RGBA8: return TSF_RGBA8;
		case ETextureSourceFormat::TSF_RGBE8: return TSF_RGBE8;
		case ETextureSourceFormat::TSF_G16: return TSF_G16;
		case ETextureSourceFormat::TSF_MAX: return TSF_MAX;
		default: return FString("NULL");
		}
	}

	static FString EnumToString(const EPixelFormat& InEnumType)
	{
		switch (InEnumType)
		{
			FOREACH_ENUM_EPIXELFORMAT(CASE_ENUM_TO_TEXT)
		}
		return TEXT("NULL");
	}

	template<typename TextureType>
	static void UpdateTexturesTable(TArray<TextureType*>& InTextures, TArray<FSceneTextureDataSet>& TargetTexturesTable, bool bOutTexturesIndices = false, TArray<int32>* OutTexturesIndices = nullptr)
	{
		for (TArray<TextureType*>::TIterator It(InTextures); It; ++It)
		{
			TextureType* InTexture = (*It);
			if (!InTexture) continue;

			FExporterHelper::FSceneTextureDataSet TextureDataSet;
			
			TextureDataSet.UniqueId = InTexture->GetUniqueID();
			TextureDataSet.Name = InTexture->GetName();
			TextureDataSet.AssetPath = InTexture->GetPathName();
			TextureDataSet.CompressionNoAlpha = InTexture->CompressionNoAlpha;
			TextureDataSet.SourceSizeX = InTexture->Source.GetSizeX();
			TextureDataSet.SourceSizeY = InTexture->Source.GetSizeY();
			TextureDataSet.SourceSize = FString::FromInt(TextureDataSet.SourceSizeX) + FString(TEXT("x")) + FString::FromInt(TextureDataSet.SourceSizeY);
			TextureDataSet.SourceFormat = FExporterHelper::EnumToStringEx(InTexture->Source.GetFormat());
			TextureDataSet.NumRefs = 1;

			TextureDataSet.LODBias = InTexture->GetCachedLODBias();
			TextureDataSet.CurrentKB = InTexture->CalcTextureMemorySizeEnum(ETextureMipCount::TMC_ResidentMips) / 1024.0f;
			TextureDataSet.FullyLoadedKB = InTexture->CalcTextureMemorySizeEnum(ETextureMipCount::TMC_AllMipsBiased) / 1024.0f;

			// Do Cast...
			UTexture2D* Texture2D = Cast<UTexture2D>(InTexture);
			if (Texture2D)
			{
				// Calculate in game current dimensions 
				const int32 DroppedMips = Texture2D->GetNumMips() - Texture2D->GetNumResidentMips();
				TextureDataSet.CurrentSizeX = Texture2D->GetSizeX() >> DroppedMips;
				TextureDataSet.CurrentSizeY = Texture2D->GetSizeY() >> DroppedMips;

				/// TextureDataSet.CurrentSizeX = Texture2D->GetSizeX() >> TextureDataSet.LODBias;
				/// TextureDataSet.CurrentSizeY = Texture2D->GetSizeY() >> TextureDataSet.LODBias;

				TextureDataSet.CurrentSize = FString::FromInt(TextureDataSet.CurrentSizeX) + FString(TEXT("x")) + FString::FromInt(TextureDataSet.CurrentSizeY);
				TextureDataSet.PixelFormat = FExporterHelper::EnumToStringEx(Texture2D->GetPixelFormat());
				TextureDataSet.Type = TEXT("2D");
				TextureDataSet.CurrentMips = Texture2D->GetNumMips();

				TextureDataSet.NumResidentMips = Texture2D->GetNumResidentMips();
				TextureDataSet.NumMipsAllowed = Texture2D->GetNumMipsAllowed(false);

				/// Texture2D->GetMinTextureResidentMipCount();
				/// Texture2D->GetNumMipsForStreaming();
				/// Texture2D->GetNumNonStreamingMips();
				/// Texture2D->GetNumRequestedMips();			
			}
			else
			{
				UTextureCube* TextureCube = Cast<UTextureCube>(InTexture);
				if (TextureCube)
				{
					TextureDataSet.CurrentSizeX = TextureCube->GetSizeX() >> TextureDataSet.LODBias;
					TextureDataSet.CurrentSizeY = TextureCube->GetSizeY() >> TextureDataSet.LODBias;
					TextureDataSet.CurrentSize = FString::FromInt(TextureDataSet.CurrentSizeX) + FString(TEXT("x")) + FString::FromInt(TextureDataSet.CurrentSizeY);
					TextureDataSet.PixelFormat = FExporterHelper::EnumToStringEx(TextureCube->GetPixelFormat());
					TextureDataSet.Type = TEXT("Cube");
					TextureDataSet.CurrentMips = TextureCube->GetNumMips();

					TextureDataSet.NumResidentMips = TextureCube->GetNumMips();
					TextureDataSet.NumMipsAllowed = TextureCube->GetNumMips();
				}
			}

			// Special Format Size...
			TextureDataSet.PVRTC2 = ::CalcTextureSize(TextureDataSet.CurrentSizeX, TextureDataSet.CurrentSizeY, EPixelFormat::PF_PVRTC2, TextureDataSet.NumResidentMips) / 1024.0f;
			TextureDataSet.PVRTC4 = ::CalcTextureSize(TextureDataSet.CurrentSizeX, TextureDataSet.CurrentSizeY, EPixelFormat::PF_PVRTC4, TextureDataSet.NumResidentMips) / 1024.0f;

			TextureDataSet.ASTC_4x4 = ::CalcTextureSize(TextureDataSet.CurrentSizeX, TextureDataSet.CurrentSizeY, EPixelFormat::PF_ASTC_4x4, TextureDataSet.NumResidentMips) / 1024.0f;
			TextureDataSet.ASTC_6x6 = ::CalcTextureSize(TextureDataSet.CurrentSizeX, TextureDataSet.CurrentSizeY, EPixelFormat::PF_ASTC_6x6, TextureDataSet.NumResidentMips) / 1024.0f;
			TextureDataSet.ASTC_8x8 = ::CalcTextureSize(TextureDataSet.CurrentSizeX, TextureDataSet.CurrentSizeY, EPixelFormat::PF_ASTC_8x8, TextureDataSet.NumResidentMips) / 1024.0f;
			TextureDataSet.ASTC_10x10 = ::CalcTextureSize(TextureDataSet.CurrentSizeX, TextureDataSet.CurrentSizeY, EPixelFormat::PF_ASTC_10x10, TextureDataSet.NumResidentMips) / 1024.0f;
			TextureDataSet.ASTC_12x12 = ::CalcTextureSize(TextureDataSet.CurrentSizeX, TextureDataSet.CurrentSizeY, EPixelFormat::PF_ASTC_12x12, TextureDataSet.NumResidentMips) / 1024.0f;

			int32 IndexTex = -1;
			/// IndexTex = TargetTexturesTable.AddUnique(TextureDataSet);

			if (TargetTexturesTable.Find(TextureDataSet, IndexTex))
			{
				TargetTexturesTable[IndexTex].NumRefs++;
			}
			else
			{
				IndexTex = TargetTexturesTable.Add(TextureDataSet);
			}

			if (bOutTexturesIndices && OutTexturesIndices)
				OutTexturesIndices->Add(IndexTex);
		}
	}

	static void GetRepresentativeInstructionCounts(TArray<FMaterialStatsUtils::FShaderInstructionsInfo>& Results, const class FMaterialResource* MaterialResource)
	{
		TMap<FName, TArray<FMaterialStatsUtils::FRepresentativeShaderInfo>> ShaderTypeNamesAndDescriptions;
		Results.Empty();

		//when adding a shader type here be sure to update FPreviewMaterial::ShouldCache()
		//so the shader type will get compiled with preview materials
		const FMaterialShaderMap* MaterialShaderMap = MaterialResource->GetGameThreadShaderMap();
		if (MaterialShaderMap && MaterialShaderMap->IsCompilationFinalized())
		{
			FMaterialStatsUtils::GetRepresentativeShaderTypesAndDescriptions(ShaderTypeNamesAndDescriptions, MaterialResource);

			if (MaterialResource->IsUIMaterial())
			{
				for (auto DescriptionPair : ShaderTypeNamesAndDescriptions)
				{
					auto& DescriptionArray = DescriptionPair.Value;
					for (int32 i = 0; i < DescriptionArray.Num(); ++i)
					{
						const FMaterialStatsUtils::FRepresentativeShaderInfo& ShaderInfo = DescriptionArray[i];

						FShaderType* ShaderType = FindShaderTypeByName(ShaderInfo.ShaderName);
						const int32 NumInstructions = MaterialShaderMap->GetMaxNumInstructionsForShader(ShaderType);

						FMaterialStatsUtils::FShaderInstructionsInfo Info;
						Info.ShaderType = ShaderInfo.ShaderType;
						Info.ShaderDescription = ShaderInfo.ShaderDescription;
						Info.InstructionCount = NumInstructions;

						Results.Push(Info);
					}
				}
			}
			else
			{
				for (auto DescriptionPair : ShaderTypeNamesAndDescriptions)
				{
					FVertexFactoryType* FactoryType = FindVertexFactoryType(DescriptionPair.Key);
					const FMeshMaterialShaderMap* MeshShaderMap = MaterialShaderMap->GetMeshShaderMap(FactoryType);
					if (MeshShaderMap)
					{
						TMap<FName, FShader*> ShaderMap;
						MeshShaderMap->GetShaderList(ShaderMap);

						auto& DescriptionArray = DescriptionPair.Value;

						for (int32 i = 0; i < DescriptionArray.Num(); ++i)
						{
							const FMaterialStatsUtils::FRepresentativeShaderInfo& ShaderInfo = DescriptionArray[i];

							FShader** ShaderEntry = ShaderMap.Find(ShaderInfo.ShaderName);
							if (ShaderEntry != nullptr)
							{
								FShaderType* ShaderType = (*ShaderEntry)->GetType();
								{
									const int32 NumInstructions = MeshShaderMap->GetMaxNumInstructionsForShader(ShaderType);

									FMaterialStatsUtils::FShaderInstructionsInfo Info;
									Info.ShaderType = ShaderInfo.ShaderType;
									Info.ShaderDescription = ShaderInfo.ShaderDescription;
									Info.InstructionCount = NumInstructions;

									Results.Push(Info);
								}
							}
						}
					}
				}
			}
		}
	}

	static void GetMatertialStatsInfo(TArray<FMaterialStatsUtils::FShaderInstructionsInfo>& ShaderInstructionInfo, FShaderStatsInfo& OutInfo, const FMaterialResource* MaterialResource)
	{
		// extract potential errors
		const ERHIFeatureLevel::Type MaterialFeatureLevel = MaterialResource->GetFeatureLevel();
		FString FeatureLevelName;
		GetFeatureLevelName(MaterialFeatureLevel, FeatureLevelName);

		OutInfo.Empty();
		TArray<FString> CompileErrors = MaterialResource->GetCompileErrors();
		for (int32 ErrorIndex = 0; ErrorIndex < CompileErrors.Num(); ErrorIndex++)
		{
			OutInfo.StrShaderErrors += FString::Printf(TEXT("[%s] %s\n"), *FeatureLevelName, *CompileErrors[ErrorIndex]);
		}

		bool bNoErrors = OutInfo.StrShaderErrors.Len() == 0;

		if (bNoErrors)
		{
			// extract instructions info
			GetRepresentativeInstructionCounts(ShaderInstructionInfo, MaterialResource);

			for (int32 InstructionIndex = 0; InstructionIndex < ShaderInstructionInfo.Num(); InstructionIndex++)
			{
				FShaderStatsInfo::FContent Content;

				Content.StrDescription = ShaderInstructionInfo[InstructionIndex].InstructionCount > 0 ? FString::Printf(TEXT("%u"), ShaderInstructionInfo[InstructionIndex].InstructionCount) : TEXT("n/a");
				Content.StrDescriptionLong = ShaderInstructionInfo[InstructionIndex].InstructionCount > 0 ?
					FString::Printf(TEXT("%s: %u instructions"), *ShaderInstructionInfo[InstructionIndex].ShaderDescription, ShaderInstructionInfo[InstructionIndex].InstructionCount) :
					TEXT("Offline shader compiler not available or an error was encountered!");

				OutInfo.ShaderInstructionCount.Add(ShaderInstructionInfo[InstructionIndex].ShaderType, Content);
			}

			// extract samplers info
			const int32 SamplersUsed = FMath::Max(MaterialResource->GetSamplerUsage(), 0);
			const int32 MaxSamplers = GetExpectedFeatureLevelMaxTextureSamplers(MaterialResource->GetFeatureLevel());
			OutInfo.SamplersCount.StrDescription = FString::Printf(TEXT("%u/%u"), SamplersUsed, MaxSamplers);
			OutInfo.SamplersCount.StrDescriptionLong = FString::Printf(TEXT("%s samplers: %u/%u"), TEXT("Texture"), SamplersUsed, MaxSamplers);

			// extract esimated sample info
			uint32 NumVSTextureSamples = 0, NumPSTextureSamples = 0;
			MaterialResource->GetEstimatedNumTextureSamples(NumVSTextureSamples, NumPSTextureSamples);

			OutInfo.TextureSampleCount.StrDescription = FString::Printf(TEXT("VS(%u), PS(%u)"), NumVSTextureSamples, NumPSTextureSamples);
			OutInfo.TextureSampleCount.StrDescriptionLong = FString::Printf(TEXT("Texture Lookups (Est.): Vertex(%u), Pixel(%u)"), NumVSTextureSamples, NumPSTextureSamples);

			// extract estimated VT info
			const uint32 NumVirtualTextureLookups = MaterialResource->GetEstimatedNumVirtualTextureLookups();
			OutInfo.VirtualTextureLookupCount.StrDescription = FString::Printf(TEXT("%u"), NumVirtualTextureLookups);
			OutInfo.VirtualTextureLookupCount.StrDescriptionLong = FString::Printf(TEXT("Virtual Texture Lookups (Est.): %u"), NumVirtualTextureLookups);

			// extract interpolators info
			uint32 UVScalarsUsed, CustomInterpolatorScalarsUsed;
			MaterialResource->GetUserInterpolatorUsage(UVScalarsUsed, CustomInterpolatorScalarsUsed);

			const uint32 TotalScalars = UVScalarsUsed + CustomInterpolatorScalarsUsed;
			const uint32 MaxScalars = FMath::DivideAndRoundUp(TotalScalars, 4u) * 4;

			OutInfo.InterpolatorsCount.StrDescription = FString::Printf(TEXT("%u/%u"), TotalScalars, MaxScalars);
			OutInfo.InterpolatorsCount.StrDescriptionLong = FString::Printf(TEXT("User interpolators: %u/%u Scalars (%u/4 Vectors) (TexCoords: %i, Custom: %i)"),
				TotalScalars, MaxScalars, MaxScalars / 4, UVScalarsUsed, CustomInterpolatorScalarsUsed);
		}
	}

	static void PrintStaticMeshesTableToCSVString(TArray<FSceneStaticMeshDataSet>& InStaticMeshesTable, TMap<FString, FString>& OutCSVStrings)
	{
		// StaticMeshesTable...
		if (InStaticMeshesTable.IsValidIndex(0))
		{
			TArray<FSceneStaticMeshDataSet>& SMDataSet = InStaticMeshesTable;

			FString ToCSVFile;
			ToCSVFile += TEXT("Id,"); ToCSVFile += TEXT("Name,"); ToCSVFile += TEXT("OwnerName,");
			ToCSVFile += TEXT("NumVertices,"); ToCSVFile += TEXT("NumTriangles,"); ToCSVFile += TEXT("NumInstances,");
			ToCSVFile += TEXT("NumLODs,"); ToCSVFile += TEXT("CurrentLOD,"); ToCSVFile += TEXT("AssetPath,");
			ToCSVFile += TEXT("UniqueId,"); ToCSVFile += TEXT("BoundsIds,"); ToCSVFile += TEXT("TransformsIds,");
			ToCSVFile += TEXT("UsedMaterialsIds,"); ToCSVFile += TEXT("UsedMaterialIntancesIds\n");
			for (int32 i = 0; i < SMDataSet.Num(); ++i)
			{
				ToCSVFile += FString::FromInt(i) + ",";
				ToCSVFile += SMDataSet[i].Name + ",";
				ToCSVFile += SMDataSet[i].OwnerName + ",";
				ToCSVFile += FString::FromInt(SMDataSet[i].NumVertices) + ",";
				ToCSVFile += FString::FromInt(SMDataSet[i].NumTriangles) + ",";
				ToCSVFile += FString::FromInt(SMDataSet[i].NumInstances) + ",";
				ToCSVFile += FString::FromInt(SMDataSet[i].NumLODs) + ",";
				ToCSVFile += FString::FromInt(SMDataSet[i].CurrentLOD) + ",";
				ToCSVFile += SMDataSet[i].AssetPath + ",";
				ToCSVFile += FString::FromInt(SMDataSet[i].UniqueId) + ",";
				for (TArray<int32>::TIterator It(SMDataSet[i].BoundsIndices); It; ++It)
					ToCSVFile += "\\" + FString::FromInt(*It);
				ToCSVFile += ",";
				for (TArray<int32>::TIterator It(SMDataSet[i].TransformsIndices); It; ++It)
					ToCSVFile += "\\" + FString::FromInt(*It);
				ToCSVFile += ",";
				for (TArray<int32>::TIterator It(SMDataSet[i].UsedMaterialsIndices); It; ++It)
					ToCSVFile += "\\" + FString::FromInt(*It);
				ToCSVFile += ",";
				for (TArray<int32>::TIterator It(SMDataSet[i].UsedMaterialIntancesIndices); It; ++It)
					ToCSVFile += "\\" + FString::FromInt(*It);
				ToCSVFile += "\n";
			}

			OutCSVStrings.Add("StaticMeshesTable", ToCSVFile);
		}
	}

	static void PrintSkeletalMeshesTableToCSVString(TArray<FSceneSkeletalMeshDataSet>& InSkeletalMeshesTable, TMap<FString, FString>& OutCSVStrings)
	{
		// SkeletalMeshesTable...
		if (InSkeletalMeshesTable.IsValidIndex(0))
		{
			TArray<FSceneSkeletalMeshDataSet>& SKDataSet = InSkeletalMeshesTable;

			FString ToCSVFile;
			ToCSVFile += TEXT("Id,"); ToCSVFile += TEXT("Name,"); ToCSVFile += TEXT("OwnerName,");
			ToCSVFile += TEXT("NumVertices,"); ToCSVFile += TEXT("NumTriangles,"); ToCSVFile += TEXT("NumSections,");
			ToCSVFile += TEXT("NumLODs,"); ToCSVFile += TEXT("CurrentLOD,"); ToCSVFile += TEXT("AssetPath,");
			ToCSVFile += TEXT("UniqueId,"); ToCSVFile += TEXT("BoundsIds,"); ToCSVFile += TEXT("TransformsIds,");
			ToCSVFile += TEXT("UsedMaterialsIds,"); ToCSVFile += TEXT("UsedMaterialIntancesIds\n");
			for (int32 i = 0; i < SKDataSet.Num(); ++i)
			{
				ToCSVFile += FString::FromInt(i) + ",";
				ToCSVFile += SKDataSet[i].Name + ",";
				ToCSVFile += SKDataSet[i].OwnerName + ",";
				ToCSVFile += FString::FromInt(SKDataSet[i].NumVertices) + ",";
				ToCSVFile += FString::FromInt(SKDataSet[i].NumTriangles) + ",";
				ToCSVFile += FString::FromInt(SKDataSet[i].NumSections) + ",";
				ToCSVFile += FString::FromInt(SKDataSet[i].NumLODs) + ",";
				ToCSVFile += FString::FromInt(SKDataSet[i].CurrentLOD) + ",";
				ToCSVFile += SKDataSet[i].AssetPath + ",";
				ToCSVFile += FString::FromInt(SKDataSet[i].UniqueId) + ",";
				ToCSVFile += "\\" + FString::FromInt(SKDataSet[i].BoundsIndex) + ",";
				ToCSVFile += "\\" + FString::FromInt(SKDataSet[i].TransformsIndex) + ",";
				for (TArray<int32>::TIterator It(SKDataSet[i].UsedMaterialsIndices); It; ++It)
					ToCSVFile += "\\" + FString::FromInt(*It);
				ToCSVFile += ",";
				for (TArray<int32>::TIterator It(SKDataSet[i].UsedMaterialIntancesIndices); It; ++It)
					ToCSVFile += "\\" + FString::FromInt(*It);
				ToCSVFile += "\n";
			}

			OutCSVStrings.Add("SkeletalMeshesTable", ToCSVFile);
		}
	}

	static void PrintLandscapesTableToCSVString(TArray<FSceneLandscapeDataSet>& InLandscapesTable, TMap<FString, FString>& OutCSVStrings) 
	{
		// LandscapesTable...
		if (InLandscapesTable.IsValidIndex(0))
		{
			FString ToCSVFile;
			ToCSVFile.Empty();
			OutCSVStrings.Add("LandscapesTable", ToCSVFile);
		}
	}

	static void PrintPrimitiveTransformsToCSVString(TArray<FMatrix>& InPrimitiveTransforms, TMap<FString, FString>& OutCSVStrings) 
	{
		// PrimitiveTransforms...
		if (InPrimitiveTransforms.IsValidIndex(0))
		{
			TArray<FMatrix>& PrimTrans = InPrimitiveTransforms;

			FString ToCSVFile;
			ToCSVFile += TEXT("Id,");
			for (int i = 0; i < 15; ++i)
				ToCSVFile += FString::FromInt(i) + ",";
			ToCSVFile += FString::FromInt(15) + "\n";
			for (int32 i = 0; i < PrimTrans.Num(); ++i)
			{
				ToCSVFile += FString::FromInt(i) + ",";
				for (int j = 0; j < 15; ++j)
					ToCSVFile += FString::SanitizeFloat(PrimTrans[i].M[j / 4][j % 4]) + ",";
				ToCSVFile += FString::SanitizeFloat(PrimTrans[i].M[3][3]) + "\n";
			}

			OutCSVStrings.Add("PrimitiveTransforms", ToCSVFile);
		}
	}

	static void PrintBoundsTableToCSVString(TArray<FBoxSphereBounds>& InBoundsTable, TMap<FString, FString>& OutCSVStrings) 
	{
		// BoundsTable...
		if (InBoundsTable.IsValidIndex(0))
		{
			TArray<FBoxSphereBounds>& Bounds = InBoundsTable;

			FString ToCSVFile;
			ToCSVFile += TEXT("Id,");
			ToCSVFile += TEXT("OriginX,"); ToCSVFile += TEXT("OriginY,"); ToCSVFile += TEXT("OriginZ,");
			ToCSVFile += TEXT("BoxExtentX,"); ToCSVFile += TEXT("BoxExtentY,"); ToCSVFile += TEXT("BoxExtentZ,");
			ToCSVFile += TEXT("SphereRadius\n");
			for (int32 i = 0; i < Bounds.Num(); ++i)
			{
				ToCSVFile += FString::FromInt(i) + ",";
				ToCSVFile += FString::SanitizeFloat(Bounds[i].Origin.X) + ",";
				ToCSVFile += FString::SanitizeFloat(Bounds[i].Origin.Y) + ",";
				ToCSVFile += FString::SanitizeFloat(Bounds[i].Origin.Z) + ",";
				ToCSVFile += FString::SanitizeFloat(Bounds[i].BoxExtent.X) + ",";
				ToCSVFile += FString::SanitizeFloat(Bounds[i].BoxExtent.Y) + ",";
				ToCSVFile += FString::SanitizeFloat(Bounds[i].BoxExtent.Z) + ",";
				ToCSVFile += FString::SanitizeFloat(Bounds[i].SphereRadius) + "\n";
			}

			OutCSVStrings.Add("BoundsTable", ToCSVFile);
		}
	}

	static void PrintMaterialsTableToCSVString(TArray<FSceneMaterialDataSet>& InMaterialsTable, TMap<FString, FString>& OutCSVStrings) 
	{
		// MaterialsTable...
		if (InMaterialsTable.IsValidIndex(0))
		{
			TArray<FSceneMaterialDataSet>& MatDataSet = InMaterialsTable;

			FString ToCSVFile;
			ToCSVFile += TEXT("Id,"); ToCSVFile += TEXT("Name,");
			ToCSVFile += TEXT("NumInstances,"); ToCSVFile += TEXT("NumRefs,");
			/////////////////////////
			// Stats
			ToCSVFile += TEXT("Stats Base Pass Shader Instructions,");
			ToCSVFile += TEXT("Stats Base Pass Shader With Surface Lightmap,");
			ToCSVFile += TEXT("Stats Base Pass Shader With Volumetric Lightmap,");
			ToCSVFile += TEXT("Stats Base Pass Vertex Shader,");
			ToCSVFile += TEXT("Stats Texture Samplers,");
			ToCSVFile += TEXT("Stats User Interpolators,");
			ToCSVFile += TEXT("Stats Texture Lookups (Est.),");
			ToCSVFile += TEXT("Stats Virtual Texture Lookups (Est.),");
			ToCSVFile += TEXT("Stats Shader Errors,");
			// Material
			ToCSVFile += TEXT("Material Domain,");
			ToCSVFile += TEXT("Material Blend Mode,");
			ToCSVFile += TEXT("Material Decal Blend Mode,");
			ToCSVFile += TEXT("Material Shading Model,");
			ToCSVFile += TEXT("Material Two Sided,");
			ToCSVFile += TEXT("Material Cast Ray Traced Shadows,");
			// Translucency
			ToCSVFile += TEXT("Translucency Screen Space Reflections,");
			ToCSVFile += TEXT("Translucency Contact Shadows,");
			ToCSVFile += TEXT("Translucency Lighting Mode,");
			ToCSVFile += TEXT("Translucency Directional Lighting Intensity,");
			ToCSVFile += TEXT("Translucency Apply Fogging,");
			ToCSVFile += TEXT("Translucency Compute Fog Per Pixel,");
			ToCSVFile += TEXT("Translucency Output Velocity,");
			// ^ // Advanced...
			ToCSVFile += TEXT("Translucency Render After DOF,");
			ToCSVFile += TEXT("Translucency Responsive AA,");
			ToCSVFile += TEXT("Translucency Mobile Separate Translucency,");
			ToCSVFile += TEXT("Translucency Disable Depth Test,");
			ToCSVFile += TEXT("Translucency Write Only Alpha,");
			ToCSVFile += TEXT("Translucency Allow Custom Depth Writes,");
			// Mobile
			ToCSVFile += TEXT("Mobile Use Full Precision,");
			ToCSVFile += TEXT("Mobile Use Lightmap Directionality,");
			// Forward Shading
			ToCSVFile += TEXT("Forward Shading High Quality Reflections,");
			ToCSVFile += TEXT("Forward Shading Planar Reflections,");
			/////////////////////////
			ToCSVFile += TEXT("AssetPath,"); ToCSVFile += TEXT("UniqueId,");
			ToCSVFile += TEXT("UsedTexturesIds,"); ToCSVFile += TEXT("MaterialInstancesIds\n");
			for (int32 i = 0; i < MatDataSet.Num(); ++i)
			{
				ToCSVFile += FString::FromInt(i) + ",";
				ToCSVFile += MatDataSet[i].Name + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].NumInstances) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].NumRefs) + ",";

				/////////////////////////
				// Stats
				ToCSVFile += FString::FromInt(MatDataSet[i].BPSCount) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].BPSSurfaceLightmap) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].BPSVolumetricLightmap) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].BPSVertex) + ",";
				ToCSVFile += "\"_" + MatDataSet[i].TexSamplers + "\",";
				ToCSVFile += "\"" + MatDataSet[i].UserInterpolators + "\",";
				ToCSVFile += "\"" + MatDataSet[i].TexLookups + "\",";
				ToCSVFile += "\"" + MatDataSet[i].VTLookups + "\",";
				ToCSVFile += "\"" + MatDataSet[i].ShaderErrors + "\",";
				// Material
				ToCSVFile += MatDataSet[i].MaterialDomain + ",";
				ToCSVFile += MatDataSet[i].BlendMode + ",";
				ToCSVFile += MatDataSet[i].DecalBlendMode + ",";
				ToCSVFile += MatDataSet[i].ShadingModel + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].TwoSided) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].bCastRayTracedShadows) + ",";
				// Translucency
				ToCSVFile += FString::FromInt(MatDataSet[i].bScreenSpaceReflections) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].bContactShadows) + ",";
				ToCSVFile += MatDataSet[i].TranslucencyLightingMode + ",";
				ToCSVFile += FString::SanitizeFloat(MatDataSet[i].TranslucencyDirectionalLightingIntensity) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].bUseTranslucencyVertexFog) + ","; // Apply Fogging
				ToCSVFile += FString::FromInt(MatDataSet[i].bComputeFogPerPixel) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].bOutputTranslucentVelocity) + ","; // Output Velocity
					// ^ // Advanced...
				ToCSVFile += FString::FromInt(MatDataSet[i].bEnableSeparateTranslucency) + ","; // Render After DOF
				ToCSVFile += FString::FromInt(MatDataSet[i].bEnableResponsiveAA) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].bEnableMobileSeparateTranslucency) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].bDisableDepthTest) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].bWriteOnlyAlpha) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].AllowTranslucentCustomDepthWrites) + ",";
				// Mobile
				ToCSVFile += FString::FromInt(MatDataSet[i].bUseFullPrecision) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].bUseLightmapDirectionality) + ",";
				// Forward Shading
				ToCSVFile += FString::FromInt(MatDataSet[i].bUseHQForwardReflections) + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].bUsePlanarForwardReflections) + ",";
				/////////////////////////

				ToCSVFile += MatDataSet[i].AssetPath + ",";
				ToCSVFile += FString::FromInt(MatDataSet[i].UniqueId) + ",";
				for (TArray<int32>::TIterator It(MatDataSet[i].UsedTexturesIndices); It; ++It)
					ToCSVFile += "\\" + FString::FromInt(*It);
				ToCSVFile += ",";
				for (TArray<int32>::TIterator It(MatDataSet[i].MatInsIndices); It; ++It)
					ToCSVFile += "\\" + FString::FromInt(*It);
				ToCSVFile += "\n";
			}

			OutCSVStrings.Add("MaterialsTable", ToCSVFile);
		}
	}

	static void PrintMaterialInstancesTableToCSVString(TArray<FSceneMaterialInstanceDataSet>& InMaterialInstancesTable, TMap<FString, FString>& OutCSVStrings)
	{
		// MaterialInstancesTable...
		if (InMaterialInstancesTable.IsValidIndex(0))
		{
			TArray<FSceneMaterialInstanceDataSet>& MatInsDataSet = InMaterialInstancesTable;

			FString ToCSVFile;
			ToCSVFile += TEXT("Id,"); ToCSVFile += TEXT("Name,");
			ToCSVFile += TEXT("NumRefs,");
			ToCSVFile += TEXT("ParentName,"); ToCSVFile += TEXT("ParentId,");
			ToCSVFile += TEXT("AssetPath,"); ToCSVFile += TEXT("UniqueId,");
			ToCSVFile += TEXT("UsedTexturesIds\n");
			for (int32 i = 0; i < MatInsDataSet.Num(); ++i)
			{
				ToCSVFile += FString::FromInt(i) + ",";
				ToCSVFile += MatInsDataSet[i].Name + ",";
				ToCSVFile += FString::FromInt(MatInsDataSet[i].NumRefs) + ",";
				ToCSVFile += MatInsDataSet[i].ParentName + ",";
				ToCSVFile += FString::FromInt(MatInsDataSet[i].ParentIndex) + ",";
				ToCSVFile += MatInsDataSet[i].AssetPath + ",";
				ToCSVFile += FString::FromInt(MatInsDataSet[i].UniqueId) + ",";
				for (TArray<int32>::TIterator It(MatInsDataSet[i].UsedTexturesIndices); It; ++It)
					ToCSVFile += "\\" + FString::FromInt(*It);
				ToCSVFile += "\n";
			}

			OutCSVStrings.Add("MaterialInstancesTable", ToCSVFile);
		}
	}

	static void PrintTexturesTableToCSVString(TArray<FSceneTextureDataSet>& InTexturesTable, TMap<FString, FString>& OutCSVStrings) 
	{
		// TexturesTable...
		if (InTexturesTable.IsValidIndex(0))
		{
			TArray<FSceneTextureDataSet>& TexDataSet = InTexturesTable;

			FString ToCSVFile;
			ToCSVFile += TEXT("Id,"); ToCSVFile += TEXT("Name,");
			ToCSVFile += TEXT("Type,"); ToCSVFile += TEXT("NumRefs,");
			ToCSVFile += TEXT("CurrentSize,"); ToCSVFile += TEXT("PixelFormat,");
			ToCSVFile += TEXT("CurrentKB,"); ToCSVFile += TEXT("FullyLoadedKB,");

			// Special Format Size...
			ToCSVFile += TEXT("PVRTC2 (KB),");
			ToCSVFile += TEXT("PVRTC4 (KB),");
			ToCSVFile += TEXT("ASTC_4x4 (KB),");
			ToCSVFile += TEXT("ASTC_6x6 (KB),");
			ToCSVFile += TEXT("ASTC_8x8 (KB),");
			ToCSVFile += TEXT("ASTC_10x10 (KB),");
			ToCSVFile += TEXT("ASTC_12x12 (KB),");

			ToCSVFile += TEXT("SourceSize,"); ToCSVFile += TEXT("SourceFormat,");
			ToCSVFile += TEXT("Compression Without Alpha,");
			ToCSVFile += TEXT("LODBias,");
			ToCSVFile += TEXT("NumResidentMips,"); ToCSVFile += TEXT("NumMipsAllowed,");
			ToCSVFile += TEXT("CurrentMips,"); 
			ToCSVFile += TEXT("CurrentSizeX,"); ToCSVFile += TEXT("CurrentSizeY,");
			ToCSVFile += TEXT("SourceSizeX,"); ToCSVFile += TEXT("SourceSizeY,");
			ToCSVFile += TEXT("AssetPath,"); ToCSVFile += TEXT("UniqueId\n");
			for (int32 i = 0; i < TexDataSet.Num(); ++i)
			{
				ToCSVFile += FString::FromInt(i) + ",";
				ToCSVFile += TexDataSet[i].Name + ",";
				ToCSVFile += TexDataSet[i].Type + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].NumRefs) + ",";
				ToCSVFile += TexDataSet[i].CurrentSize + ",";
				ToCSVFile += TexDataSet[i].PixelFormat + ",";
				ToCSVFile += FString::SanitizeFloat(TexDataSet[i].CurrentKB) + ",";
				ToCSVFile += FString::SanitizeFloat(TexDataSet[i].FullyLoadedKB) + ",";

				// Special Format Size...
				ToCSVFile += FString::SanitizeFloat(TexDataSet[i].PVRTC2) + ",";
				ToCSVFile += FString::SanitizeFloat(TexDataSet[i].PVRTC4) + ",";
				ToCSVFile += FString::SanitizeFloat(TexDataSet[i].ASTC_4x4) + ",";
				ToCSVFile += FString::SanitizeFloat(TexDataSet[i].ASTC_6x6) + ",";
				ToCSVFile += FString::SanitizeFloat(TexDataSet[i].ASTC_8x8) + ",";
				ToCSVFile += FString::SanitizeFloat(TexDataSet[i].ASTC_10x10) + ",";
				ToCSVFile += FString::SanitizeFloat(TexDataSet[i].ASTC_12x12) + ",";

				ToCSVFile += TexDataSet[i].SourceSize + ",";
				ToCSVFile += TexDataSet[i].SourceFormat + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].CompressionNoAlpha) + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].LODBias) + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].NumResidentMips) + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].NumMipsAllowed) + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].CurrentMips) + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].CurrentSizeX) + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].CurrentSizeY) + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].SourceSizeX) + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].SourceSizeY) + ",";
				ToCSVFile += TexDataSet[i].AssetPath + ",";
				ToCSVFile += FString::FromInt(TexDataSet[i].UniqueId) + "\n";
			}

			OutCSVStrings.Add("TexturesTable", ToCSVFile);
		}
	}

	static void PrintSceneDataSetToCSVString(FSceneDataSet& InSceneDataSet, TMap<FString, FString>& OutCSVStrings)
	{
		PrintStaticMeshesTableToCSVString(InSceneDataSet.StaticMeshesTable, OutCSVStrings);
		PrintSkeletalMeshesTableToCSVString(InSceneDataSet.SkeletalMeshesTable, OutCSVStrings);
		PrintLandscapesTableToCSVString(InSceneDataSet.LandscapesTable, OutCSVStrings);

		PrintPrimitiveTransformsToCSVString(InSceneDataSet.PrimitiveTransforms, OutCSVStrings);
		PrintBoundsTableToCSVString(InSceneDataSet.BoundsTable, OutCSVStrings);
		PrintMaterialsTableToCSVString(InSceneDataSet.MaterialsTable, OutCSVStrings);
		PrintMaterialInstancesTableToCSVString(InSceneDataSet.MaterialInstancesTable, OutCSVStrings);
		PrintTexturesTableToCSVString(InSceneDataSet.TexturesTable, OutCSVStrings);
	}

	/** Main Entry Second... */
	static void ExportSceneDataToCSV(FScene* InScene, TMap<FPrimitiveComponentId, UPrimitiveComponent*>& InPrimitivesTable, TMap<FString, bool>& OutResultPathsStates, const FString& InOutputPath, const FString& InTablePrefix)
	{
		if (InScene && InScene->PrimitiveComponentIds.IsValidIndex(0))
		{
			TArray<FExporterHelper::FSceneDataSet> PerLODSceneDataSets;
			// Default array num is 1...And this is the base data set of all...
			uint16 MaxLODs = 1;
			PerLODSceneDataSets.AddZeroed(1);

			for (TArray<FPrimitiveComponentId>::TIterator It_0(InScene->PrimitiveComponentIds); It_0; ++It_0)
			{
				FPrimitiveComponentId InPrimitiveComponentId = (*It_0);

				if (InPrimitivesTable.Contains(InPrimitiveComponentId))
				{
					UPrimitiveComponent* InPrimitiveComponent = InPrimitivesTable[InPrimitiveComponentId];
					if (!InPrimitiveComponent) continue;

					TArray<UMaterialInterface*> UsedMaterials;

					int32		  BoundsIndex = -1;
					int32		  TransformsIndex = -1;
					TArray<int32> UsedMaterialsIndices;
					TArray<int32> UsedMaterialIntancesIndices;

					// Do Cast...
					UStaticMeshComponent*	StaticMeshComponent = Cast<UStaticMeshComponent>(InPrimitiveComponent);
					USkeletalMeshComponent*	SkeletalMeshComponent = Cast<USkeletalMeshComponent>(InPrimitiveComponent);
					ULandscapeComponent*	LandscapeComponent = Cast<ULandscapeComponent>(InPrimitiveComponent);

					// Instanced data...
					UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(InPrimitiveComponent);
					// BSP geometry... Not Care Now...
					UModelComponent* ModelComponent = Cast<UModelComponent>(InPrimitiveComponent);

					// Fill table of...Mat, MatIns, Tex, Bound, Trans...
					if (StaticMeshComponent || SkeletalMeshComponent || LandscapeComponent)
					{
						BoundsIndex = PerLODSceneDataSets[0].BoundsTable.Num();
						PerLODSceneDataSets[0].BoundsTable.Add(InPrimitiveComponent->Bounds); // Need to be Confirmed...

						TransformsIndex = PerLODSceneDataSets[0].PrimitiveTransforms.Num();
						PerLODSceneDataSets[0].PrimitiveTransforms.Add(InPrimitiveComponent->GetRenderMatrix());

						InPrimitiveComponent->GetUsedMaterials(UsedMaterials);
						for (TArray<UMaterialInterface*>::TIterator It_1(UsedMaterials); It_1; ++It_1)
						{
							UMaterialInterface* InMaterial = (*It_1);
							if (!InMaterial) continue;

							// Do Cast...
							UMaterial*			  Material = Cast<UMaterial>(InMaterial);
							UMaterialInstance* MaterialIns = Cast<UMaterialInstance>(InMaterial);

							TArray<int32> UsedTexturesIndices;
							TArray<UTexture*> UsedTextures;
							InMaterial->GetUsedTextures(UsedTextures, EMaterialQualityLevel::Num, false, GMaxRHIFeatureLevel, true);
							FExporterHelper::UpdateTexturesTable<UTexture>(UsedTextures, PerLODSceneDataSets[0].TexturesTable, true, &UsedTexturesIndices);

							FExporterHelper::FSceneMaterialDataSet MaterialDataSet;
							if (Material)
							{
								MaterialDataSet.Init(Material, UsedTexturesIndices);

								int32 IndexMat = -1;
								/// IndexMat = PerLODSceneDataSets[0].MaterialsTable.AddUnique(MaterialDataSet);

								if (PerLODSceneDataSets[0].MaterialsTable.Find(MaterialDataSet, IndexMat))
								{
									PerLODSceneDataSets[0].MaterialsTable[IndexMat].NumRefs++;
								}
								else
								{
									IndexMat = PerLODSceneDataSets[0].MaterialsTable.Add(MaterialDataSet);
								}

								UsedMaterialsIndices.Add(IndexMat);
							}
							else if (MaterialIns)
							{
								FExporterHelper::FSceneMaterialInstanceDataSet MaterialInsDataSet;

								MaterialInsDataSet.UniqueId = MaterialIns->GetUniqueID();
								MaterialInsDataSet.Name = MaterialIns->GetName();
								MaterialInsDataSet.AssetPath = MaterialIns->GetPathName();
								MaterialInsDataSet.UsedTexturesIndices = UsedTexturesIndices;
								MaterialInsDataSet.ParentName = MaterialIns->Parent->GetName();

								FExporterHelper::FSceneMaterialDataSet MatInsParent;
								uint32 TempUniqueId = MaterialIns->Parent->GetUniqueID();
								MatInsParent.UniqueId = MaterialIns->Parent->GetMaterial()->GetUniqueID();

								// Parent Mat Relevance...
								int32 ParentIndex = -1;
								if (PerLODSceneDataSets[0].MaterialsTable.Find(MatInsParent, ParentIndex))
								{
									PerLODSceneDataSets[0].MaterialsTable[ParentIndex].NumRefs++;
								}
								else
								{
									// @todo...When add new Mat to Table...Update Tex Table data...
									TArray<int32> ParentMatUsedTexIndices;
									TArray<UTexture*> NewMatUsedTextures;
									MaterialIns->Parent->GetMaterial()->GetUsedTextures(NewMatUsedTextures, EMaterialQualityLevel::Num, false, GMaxRHIFeatureLevel, true);
									FExporterHelper::UpdateTexturesTable<UTexture>(NewMatUsedTextures, PerLODSceneDataSets[0].TexturesTable, true, &ParentMatUsedTexIndices);
									MatInsParent.Init(MaterialIns->Parent->GetMaterial(), ParentMatUsedTexIndices);
									// After...
									ParentIndex = PerLODSceneDataSets[0].MaterialsTable.Add(MatInsParent);
								}
								MaterialInsDataSet.ParentIndex = ParentIndex;

								int32 IndexMatIns = -1;
								/// IndexMatIns = PerLODSceneDataSets[0].MaterialInstancesTable.AddUnique(MaterialInsDataSet);

								if (PerLODSceneDataSets[0].MaterialInstancesTable.Find(MaterialInsDataSet, IndexMatIns))
								{
									PerLODSceneDataSets[0].MaterialInstancesTable[IndexMatIns].NumRefs++;
								}
								else
								{
									MaterialInsDataSet.NumRefs = 1;
									IndexMatIns = PerLODSceneDataSets[0].MaterialInstancesTable.Add(MaterialInsDataSet);
								}

								PerLODSceneDataSets[0].MaterialsTable[ParentIndex].NumInstances++;
								PerLODSceneDataSets[0].MaterialsTable[ParentIndex].MatInsIndices.AddUnique(IndexMatIns); // Use Add to Repeated marks
								UsedMaterialIntancesIndices.Add(IndexMatIns);
							}
						}
					}

					// ...If...
					if (StaticMeshComponent)
					{
						UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
						if (StaticMesh && StaticMesh->RenderData)
						{
							FExporterHelper::FSceneStaticMeshDataSet StaticMeshDataSet;
							StaticMeshDataSet.UniqueId = StaticMesh->GetUniqueID();
							StaticMeshDataSet.Name = StaticMesh->GetName();
							StaticMeshDataSet.AssetPath = StaticMesh->GetPathName();
							StaticMeshDataSet.OwnerName = StaticMeshComponent->GetOwner()->GetName();
							// Remove xxx_number...
							int32 FindLastIndex = -1; // INDEX_NONE
							if (StaticMeshDataSet.OwnerName.FindLastChar('_', FindLastIndex))
								StaticMeshDataSet.OwnerName.RemoveAt(FindLastIndex, StaticMeshDataSet.OwnerName.Len() - FindLastIndex);

							// Init First Elem...
							StaticMeshDataSet.BoundsIndices.AddZeroed(1);
							StaticMeshDataSet.TransformsIndices.AddZeroed(1);
							// First is Mesh...Rest is Instance...
							StaticMeshDataSet.BoundsIndices[0] = BoundsIndex;
							StaticMeshDataSet.TransformsIndices[0] = TransformsIndex;

							StaticMeshDataSet.UsedMaterialsIndices = UsedMaterialsIndices;
							StaticMeshDataSet.UsedMaterialIntancesIndices = UsedMaterialIntancesIndices;

							// Fill Bounds, Trans Ins...
							StaticMeshDataSet.NumInstances = 0;
							if (InstancedStaticMeshComponent)
							{
								StaticMeshDataSet.NumInstances = InstancedStaticMeshComponent->PerInstanceSMData.Num();
								FBoxSphereBounds CurrentInsMeshBounds = StaticMesh->GetBounds();
								for (uint32 i = 0; i < StaticMeshDataSet.NumInstances; ++i)
								{
									FMatrix TransIns = InstancedStaticMeshComponent->PerInstanceSMData[i].Transform;
									CurrentInsMeshBounds.Origin = FVector(TransIns.M[3][0], TransIns.M[3][1], TransIns.M[3][2]);

									BoundsIndex = PerLODSceneDataSets[0].BoundsTable.Num();
									StaticMeshDataSet.BoundsIndices.Add(BoundsIndex);
									PerLODSceneDataSets[0].BoundsTable.Add(CurrentInsMeshBounds);

									TransformsIndex = PerLODSceneDataSets[0].PrimitiveTransforms.Num();
									StaticMeshDataSet.TransformsIndices.Add(TransformsIndex);
									PerLODSceneDataSets[0].PrimitiveTransforms.Add(TransIns);
								}
							}

							uint16 LODs = StaticMesh->RenderData->LODResources.Num();
							MaxLODs = LODs < MaxLODs ? MaxLODs : LODs;
							// Adjust array num to max LODs...
							if (PerLODSceneDataSets.Num() < MaxLODs)
								PerLODSceneDataSets.AddZeroed(MaxLODs - PerLODSceneDataSets.Num());

							for (uint16 CurrentLOD = 0; CurrentLOD < LODs; ++CurrentLOD)
							{
								FStaticMeshLODResources* CurrentLODRes = &(StaticMesh->RenderData->LODResources[CurrentLOD]);

								StaticMeshDataSet.CurrentLOD = CurrentLOD;
								StaticMeshDataSet.NumLODs = LODs;
								StaticMeshDataSet.NumVertices = CurrentLODRes->GetNumVertices();
								StaticMeshDataSet.NumTriangles = CurrentLODRes->GetNumTriangles();

								PerLODSceneDataSets[CurrentLOD].StaticMeshesTable.Add(StaticMeshDataSet);
							}
						}
					}
					else if (SkeletalMeshComponent)
					{
						USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->SkeletalMesh;
						FSkeletalMeshRenderData* RenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();
						if (SkeletalMesh && RenderData)
						{
							FExporterHelper::FSceneSkeletalMeshDataSet SkeletalMeshDataSet;
							SkeletalMeshDataSet.UniqueId = SkeletalMesh->GetUniqueID();
							SkeletalMeshDataSet.Name = SkeletalMesh->GetName();
							SkeletalMeshDataSet.AssetPath = SkeletalMesh->GetPathName();
							SkeletalMeshDataSet.OwnerName = SkeletalMeshComponent->GetOwner()->GetName();
							// Remove xxx_number...
							int32 FindLastIndex = -1; // INDEX_NONE
							if (SkeletalMeshDataSet.OwnerName.FindLastChar('_', FindLastIndex))
								SkeletalMeshDataSet.OwnerName.RemoveAt(FindLastIndex, SkeletalMeshDataSet.OwnerName.Len() - FindLastIndex);

							SkeletalMeshDataSet.BoundsIndex = BoundsIndex;
							SkeletalMeshDataSet.TransformsIndex = TransformsIndex;

							SkeletalMeshDataSet.UsedMaterialsIndices = UsedMaterialsIndices;
							SkeletalMeshDataSet.UsedMaterialIntancesIndices = UsedMaterialIntancesIndices;

							uint16 LODs = RenderData->LODRenderData.Num();
							MaxLODs = LODs < MaxLODs ? MaxLODs : LODs;
							// Adjust array num to max LODs...
							if (PerLODSceneDataSets.Num() < MaxLODs)
								PerLODSceneDataSets.AddZeroed(MaxLODs - PerLODSceneDataSets.Num());

							for (uint16 CurrentLOD = 0; CurrentLOD < LODs; ++CurrentLOD)
							{
								FSkeletalMeshLODRenderData* CurrentLODRes = &(RenderData->LODRenderData[CurrentLOD]);

								SkeletalMeshDataSet.CurrentLOD = CurrentLOD;
								SkeletalMeshDataSet.NumLODs = LODs;
								SkeletalMeshDataSet.NumVertices = CurrentLODRes->GetNumVertices();
								SkeletalMeshDataSet.NumSections = CurrentLODRes->RenderSections.Num();
								SkeletalMeshDataSet.NumTriangles = 0;
								for (uint32 i = 0; i < SkeletalMeshDataSet.NumSections; ++i)
									SkeletalMeshDataSet.NumTriangles += CurrentLODRes->RenderSections[i].NumTriangles;

								PerLODSceneDataSets[CurrentLOD].SkeletalMeshesTable.Add(SkeletalMeshDataSet);
							}
						}
					}
					else if (LandscapeComponent)
					{
						LandscapeComponent->GetName();
					}
				}
			}

			// Save to CSV Files...
			for (uint16 CurrentLOD = 0; CurrentLOD < MaxLODs; ++CurrentLOD)
			{
				TMap<FString, FString> CSVStrings;
				FExporterHelper::PrintSceneDataSetToCSVString(PerLODSceneDataSets[CurrentLOD], CSVStrings);

				for (TMap<FString, FString>::TIterator It(CSVStrings); It; ++It)
				{
					FString SavedFilePath = InOutputPath + "/" + InTablePrefix + "_" + (*It).Key + "_LOD" + FString::FromInt(CurrentLOD) + ".csv";

					bool ResultsStates = FFileHelper::SaveStringToFile((*It).Value, SavedFilePath.GetCharArray().GetData(), FFileHelper::EEncodingOptions::ForceUTF8);

					OutResultPathsStates.Add(SavedFilePath, ResultsStates);
				}
			}
		}
	}

	/** Main Entry First... */
	static void ExportSceneDataToCSV(TMap<FString, bool>& OutResultPathsStates, const FString& InOutputPath)
	{
		UWorld* World = FExporterHelper::GetWorld();

		if (World)
		{
			TMap<ULevel*, TMap<FPrimitiveComponentId, UPrimitiveComponent*>> PerLevelComps;

			TMap<FPrimitiveComponentId, UPrimitiveComponent*> PrimitivesTable;
			// Build PrimitivesTable...
			{
				TArray<ULevel*> Levels;

				// Add main level.
				Levels.AddUnique(World->PersistentLevel);

				// Add secondary levels.
				for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
				{
					if (StreamingLevel)
					{
						if (ULevel* Level = StreamingLevel->GetLoadedLevel())
						{
							Levels.AddUnique(Level);
						}
					}
				}

				TMap<FPrimitiveComponentId, UPrimitiveComponent*> EmptyPrims; EmptyPrims.Empty();
				for (TArray<ULevel*>::TIterator It(Levels); It; ++It)
				{
					PerLevelComps.Add((*It), EmptyPrims);
				}

				// Iterate UPrimitiveComponent...
				for (TObjectIterator<UPrimitiveComponent> It; It; ++It)
				{
					AActor* Owner = (*It)->GetOwner();

					if (Owner != nullptr && !Owner->HasAnyFlags(RF_ClassDefaultObject))
					{
						ULevel* CheckLevel = Owner->GetLevel();

						if (CheckLevel != nullptr && (Levels.Contains(CheckLevel)))
						{
							UPrimitiveComponent* InPrimitiveComponent = (*It);
						
							PerLevelComps[CheckLevel].Add(InPrimitiveComponent->ComponentId, InPrimitiveComponent);
							// Build PrimitivesTable...
							PrimitivesTable.Add(InPrimitiveComponent->ComponentId, InPrimitiveComponent);
						}
					}
				}
			}

			FScene* Scene = (FScene*)World->Scene;

#if 0
			// Test Octree...
			FBox TestBox(FVector(-100.f), FVector(100.0f));
			TMap<const FScenePrimitiveOctree::FNode*, FOctreeNodeContext> IntersectNodes;

			for (FScenePrimitiveOctree::TConstIterator<> PrimitiveOctreeIt(Scene->PrimitiveOctree);
				PrimitiveOctreeIt.HasPendingNodes();
				PrimitiveOctreeIt.Advance())
			{
				const FScenePrimitiveOctree::FNode& PrimitiveOctreeNode = PrimitiveOctreeIt.GetCurrentNode();
				const FOctreeNodeContext& PrimitiveOctreeNodeContext = PrimitiveOctreeIt.GetCurrentContext();

				// Assume Root Node is always include...
				IntersectNodes.Add(&PrimitiveOctreeNode, PrimitiveOctreeNodeContext);

				{
					// Find children of this Octree node that may contain relevant primitives...
					FOREACH_OCTREE_CHILD_NODE(ChildRef)
					{
						if (PrimitiveOctreeNode.HasChild(ChildRef))
						{
							// Check that the child node is in the TestBox...
							const FOctreeNodeContext ChildContext = PrimitiveOctreeNodeContext.GetChildContext(ChildRef);

							if (TestBox.Intersect(ChildContext.Bounds.GetBox()))
							{
								// If the child node was in the TestBox, push it on the iterator's pending node stack...
								PrimitiveOctreeIt.PushChild(ChildRef);
							}
						}
					}
				}			
			}
#endif
			FString WorldName = World->GetName();
						
			ExportSceneDataToCSV(Scene, PrimitivesTable, OutResultPathsStates, InOutputPath + "/World_" + WorldName, WorldName);

			TArray<UTexture2D*> WorldTotalLitShadowMaps;
			for (TMap<ULevel*, TMap<FPrimitiveComponentId, UPrimitiveComponent*>>::TIterator It(PerLevelComps); It; ++It)
			{
				/// FString LevelName = (*It).Key->GetFullGroupName(true);
				FString LevelName = (*It).Key->GetOuter()->GetName();

				ExportSceneDataToCSV(Scene, (*It).Value, OutResultPathsStates, InOutputPath + "/" + LevelName, LevelName);

				// Export Per Level LightMaps & ShadowMaps...
				TArray<UTexture2D*> PerLevelLitShadowMaps;
				TArray<FSceneTextureDataSet> PerLevelLSTexturesTable;
				TMap<FString, FString> CSVStrings;

				World->GetLightMapsAndShadowMaps((*It).Key, PerLevelLitShadowMaps);
				WorldTotalLitShadowMaps.Append(PerLevelLitShadowMaps);
				UpdateTexturesTable<UTexture2D>(PerLevelLitShadowMaps, PerLevelLSTexturesTable);
				PrintTexturesTableToCSVString(PerLevelLSTexturesTable, CSVStrings);

				// Save to CSV Files...
				if (CSVStrings.Num() > 0)
				{
					FString SavedFilePath = InOutputPath + "/" + LevelName + "/" + LevelName + "_LightMapsAndShadowMaps.csv";
					bool ResultsStates = FFileHelper::SaveStringToFile(CSVStrings["TexturesTable"], SavedFilePath.GetCharArray().GetData(), FFileHelper::EEncodingOptions::ForceUTF8);
					OutResultPathsStates.Add(SavedFilePath, ResultsStates);
				}				
			}

			// World Total LightMaps & ShadowMaps...
			TArray<FSceneTextureDataSet> WorldTotalLSTexturesTable;
			TMap<FString, FString> TotalLSMapsCSVStrings;
			UpdateTexturesTable<UTexture2D>(WorldTotalLitShadowMaps, WorldTotalLSTexturesTable);
			PrintTexturesTableToCSVString(WorldTotalLSTexturesTable, TotalLSMapsCSVStrings);
			
			// Save to CSV Files...
			if (TotalLSMapsCSVStrings.Num() > 0)
			{
				FString SavedFilePath = InOutputPath + "/World_" + WorldName + "/" + WorldName + "_LightMapsAndShadowMaps.csv";
				bool ResultsStates = FFileHelper::SaveStringToFile(TotalLSMapsCSVStrings["TexturesTable"], SavedFilePath.GetCharArray().GetData(), FFileHelper::EEncodingOptions::ForceUTF8);
				OutResultPathsStates.Add(SavedFilePath, ResultsStates);
			}		
		}				
	}
};