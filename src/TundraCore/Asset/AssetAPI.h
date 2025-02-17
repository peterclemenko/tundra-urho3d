// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include "CoreStringUtils.h"
#include "AssetFwd.h"
#include "IAssetStorage.h"
#include "IAssetTypeFactory.h"
#include "IAssetTransfer.h"
#include "IAssetBundle.h"
#include "CoreStringUtils.h"
#include "Signals.h"

#include <Urho3D/Core/Object.h>
#include <map>
#include <string>

namespace Tundra
{

/// Loads the given local file into the specified vector. Clears all data previously in the vector.
/// Returns true on success.
bool TUNDRACORE_API LoadFileToVector(const String &filename, Vector<u8> &dst);

/// Copies the given source file to the destination file on the local filesystem. Returns true on success.
bool TUNDRACORE_API CopyAssetFile(const String &sourceFile, const String &destFile);

/// Saves the given raw data buffer to destFile. Returns true on success.
bool TUNDRACORE_API SaveAssetFromMemoryToFile(const u8 *data, uint numBytes, const String &destFile);

/// Given a string of form "someString?param1=value1&param2=value2", returns a map of key-value pairs.
/// @param body [out] If specified, the part 'someString' is returned here.
HashMap<String, String> TUNDRACORE_API ParseAssetRefArgs(const String &url, String *body);

/// Adds a trailing slash to the given string representing a directory path if it doesn't have one at the end already.
/// This trailing slash will always be of the unix format, i.e. '/'.
/// If an empty string is submitted, and empty string will be output, so that an empty string won't suddenly point to the filesystem root.
String TUNDRACORE_API GuaranteeTrailingSlash(const String &source);

typedef std::map<String, AssetPtr, StringCompareCaseInsensitive> AssetMap; ///<  Maps asset names to their AssetPtrs.
typedef std::map<String, AssetTransferPtr, StringCompareCaseInsensitive> AssetTransferMap;
typedef Vector<AssetStoragePtr> AssetStorageVector;
typedef std::map<String, AssetBundlePtr, StringCompareCaseInsensitive> AssetBundleMap; ///<  Maps asset bundle names to their AssetBundlePtrs.
typedef std::map<String, AssetBundleMonitorPtr, StringCompareCaseInsensitive> AssetBundleMonitorMap;

/// Implements asset download and upload functionality.
/** Registers LocalAssetProvider and BinaryAssetFactory by default. */
class TUNDRACORE_API AssetAPI : public Object
{
    URHO3D_OBJECT(AssetAPI, Object);

    friend class Framework;

public:
    AssetAPI(Framework *fw, bool headless);
    ~AssetAPI();

    /// Returns the given asset by full URL ref if it exists casted to the correct type T, or null otherwise.
    /** @note The "name" of an asset is in most cases the URL ref of the asset, so use this function to query an asset by name. */
    template<typename T>
    SharedPtr<T> FindAsset(String assetRef) const;

    /// Returns all assets of a specific type T (derived from IAsset).
    template<typename T>
    Vector<SharedPtr<T> > AssetsOfType() const;

    /// Set new asset transfer prioritizer.
    /** Can be used to override the DefaultAssetTransferPrioritizer implementation
        that is automatically set by AssetAPI.
        @note You can also pass a null ptr to remove any asset prioritization. */
    void SetAssetTransferPrioritizer(AssetTransferPrioritizerPtr prioritizer);

    /// Returns the current asset transfer prioritizer.
    /** The returned pointer may also be null. @see SetAssetTransferPrioritizer */
    AssetTransferPrioritizerWeakPtr AssetTransferPrioritizer() const;

    /// Registers a type factory for creating assets of the type governed by the factory.
    void RegisterAssetTypeFactory(AssetTypeFactoryPtr factory);

    /// Registers a type factory for creating asset bundles of the type governed by the factory.
    void RegisterAssetBundleTypeFactory(AssetBundleTypeFactoryPtr factory);

    /// Returns all registered asset type factories.
    /** You can use this list to query which asset types the system can handle. */
    Vector<AssetTypeFactoryPtr> AssetTypeFactories() const { return assetTypeFactories; }

    /// Returns the asset provider of the given type.
    /** The registered asset providers are unique by type. You cannot register two instances of the same provider type to the system. */
    template<typename T>
    SharedPtr<T> AssetProvider() const;

    /// Registers a new asset provider to the Asset API.
    /** Use this to add a new provider type you have instantiated to the system. */
    void RegisterAssetProvider(AssetProviderPtr provider);

    /// Returns all the asset providers that are registered to the Asset API.
    Vector<AssetProviderPtr> AssetProviders() const;

    /// Returns all the currently ongoing or waiting asset transfers.
    Vector<AssetTransferPtr> PendingTransfers() const;

    /// Performs internal tick-based updates of the whole asset system.
    /** This function is intended to be called only by the core, do not call it yourself. */
    void Update(float frametime);

    /// Called by each AssetProvider to notify the Asset API that an asset transfer has completed.
    /** Do not call this function from client code. */
    void AssetTransferCompleted(IAssetTransfer *transfer);

    /// Called by each AssetProvider to notify the Asset API that the asset transfer finished in a failure.
    /** The Asset API will erase this transfer and also fail any transfers of assets which depended on this transfer. */
    void AssetTransferFailed(IAssetTransfer *transfer, String reason);
    
    /// Called by each AssetProvider to notify of aborted transfers.
    /** The Asset API will erase this transfer and also fail any transfers of assets which depended on this transfer. */
    void AssetTransferAborted(IAssetTransfer *transfer);

    /// Called by each IAsset when it has completed loading successfully.
    /** Typically inside IAsset::DeserializeFromData or later on if it is loading asynchronously. */
    void AssetLoadCompleted(const String assetRef);

    /// Called by each IAsset when it has failed to load.
    /** Typically inside IAsset::DeserializeFromData or later on if it is loading asynchronously. */
    void AssetLoadFailed(const String assetRef);

    /// Called by each AssetProvider to notify the Asset API that an asset upload transfer has completed. Do not call this function from client code.
    void AssetUploadTransferCompleted(IAssetUploadTransfer *transfer);

    void AssetDependenciesCompleted(AssetTransferPtr transfer);

    void NotifyAssetDependenciesChanged(AssetPtr asset);

    bool IsHeadless() const { return isHeadless; }

    /// Returns all the currently loaded assets which depend on the asset dependeeAssetRef.
    Vector<AssetPtr> FindDependents(String dependeeAssetRef);

    /// Specifies the different possible results for AssetAPI::ResolveLocalAssetPath.
    enum FileQueryResult
    {
        FileQueryLocalFileFound, ///< The asset reference specified a local filesystem file, and the absolute path name for it was found.
        FileQueryLocalFileMissing, ///< The asset reference specified a local filesystem file, but there was no file in that location.
        FileQueryExternalFile ///< The asset reference points to a file in an external source, which cannot be checked for existence in the current function (would require a network lookup).
    };

    enum AssetRefType
    {
        AssetRefInvalid,
        AssetRefLocalPath,      ///< The assetRef points to the local filesystem using an *absolute* pathname, e.g "C:\myassets\texture.png".
        AssetRefRelativePath,   ///< The assetRef is a path relative to some context. "asset.png" or "relativePath/model.mesh".
        AssetRefLocalUrl,       ///< The assetRef points to the local filesystem, using a local URL syntax like local:// or file://. E.g. "local://texture.png".
        AssetRefExternalUrl,    ///< The assetRef points to a location external to the system, using a URL protocol specifier. "http://server.com/asset.png".
        AssetRefNamedStorage    ///< The assetRef points to an explicitly named storage. "storageName:asset.png".
    };

    /// Breaks the given assetRef into pieces, and returns the parsed type.
    /** @param assetRef The assetRef to parse.
        @param outProtocolPart [out]  Receives the protocol part of the ref, e.g. "http://server.com/asset.png" -> "http". If it doesn't exist, returns an empty string.
        @param outNamedStorage [out]  Receives the named storage specifier in the ref, e.g. "myStorage:asset.png" -> "myStorage". If it doesn't exist, returns an empty string.
        @param outProtocol_Path [out] Receives the "path name" identifying where the asset is stored in.
                                      e.g. "http://server.com/path/folder2/asset.png" -> "http://server.com/path/folder2/". 
                                           "myStorage:asset.png" -> "myStorage:". Always has a trailing slash if necessary.
        @param outPath_Filename_SubAssetName [out] Gets the combined path name, asset filename and asset subname in the ref.
                                      e.g. "local://path/folder/asset.zip#subAsset" -> "path/folder/asset.zip#subAsset".
                                           "namedStorage:path/folder/asset.zip#subAsset" -> "path/folder/asset.zip, subAsset".
        @param outPath_Filename [out] Gets the combined path name and asset filename in the ref.
                                      e.g. "local://path/folder/asset.zip#subAsset" -> "path/folder/asset.zip".
                                           "namedStorage:path/folder/asset.zip#subAsset" -> "path/folder/asset.zip".
        @param outPath [out] Returns the path part of the ref, e.g. "local://path/folder/asset.zip#subAsset" -> "path/folder/". Has a trailing slash when necessary.
        @param outFilename [out] Returns the base filename of the asset. e.g. "local://path/folder/asset.zip#subAsset" -> "asset.zip".
        @param outSubAssetName [out] Returns the sub asset name in the ref. e.g. "local://path/folder/asset.zip#subAsset" -> "subAsset".
        @param outFullRef [out] Returns a cleaned or "canonicalized" version of the asset ref in full.
        @param outFullRefNoSubAssetName [out] Returns a cleaned or "canonicalized" version of the asset ref in full without possible sub asset. */
    static AssetRefType ParseAssetRef(String assetRef, String *outProtocolPart = 0, String *outNamedStorage = 0, String *outProtocol_Path = 0, 
        String *outPath_Filename_SubAssetName = 0, String *outPath_Filename = 0, String *outPath = 0, String *outFilename = 0, String *outSubAssetName = 0,
        String *outFullRef = 0, String *outFullRefNoSubAssetName = 0);

    typedef Vector<Pair<String, String> > AssetDependenciesMap;
    
    /// Sanitates an assetref so that it can be used as a filename for caching.
    /** Characters like ':'. '/', '\' and '*' will be replaced with $1, $2, $3, $4 .. respectively, in a reversible way.
        Note that sanitated assetrefs will not work when querying from the asset system, for that you need the desanitated form.
        @sa DesanitateAssetRef */
    static String SanitateAssetRef(const String& ref);
    static std::string SanitateAssetRef(const std::string& ref); /**< @overload */

    /// Desanitates an assetref with $1 $2 $3 $4 ... into original form.
    /** @sa SanitateAssetRef */
    static String DesanitateAssetRef(const String& ref);
    static std::string DesanitateAssetRef(const std::string& ref); /**< @overload */

    /// Explodes the given asset storage description string to key-value pairs.
    static HashMap<String, String> ParseAssetStorageString(String storageString);

    Framework *GetFramework() const { return fw; }

    /// Returns all assets known to the asset system.
    AssetMap Assets() const { return assets; }

    /// Returns all asset bundles known to the asset system.
    AssetBundleMap AssetBundles() const { return assetBundles; }

    /// Returns all assets of a specific type.
    AssetMap AssetsOfType(const String& type) const;

    /// Returns the known asset storage instances in the system.
    AssetStorageVector AssetStorages() const;

    /// Opens the internal Asset API asset cache to the given directory.
    /** When the Asset API starts up, the asset cache is not created. This allows the Asset API to be operated in a mode that does not 
        perform writes to the disk when assets are fetched. This will cause assets fetched from remote hosts to have a null disk source.
        @note Once the asset cache has been created with a call to this function, there is no way to close the asset cache (except to close and restart). */
    void OpenAssetCache(String directory);

    /// Requests the given asset to be downloaded.
    /** The transfer will go to a pending transfers queue and will be processed when possible.
        @param assetRef The asset reference (a filename or a full URL) to request. The name of the resulting asset is the same as the asset reference
              that is used to load it.
        @param assetType The type of the asset to request. This can be null if the assetRef itself identifies the asset type.
        @param forceTransfer Force transfer even if the asset is in the loaded state
        @return A pointer to the created asset transfer, or null if the transfer could not be initiated. */
    AssetTransferPtr RequestAsset(String assetRef, String assetType = "", bool forceTransfer = false);
    AssetTransferPtr RequestAsset(const AssetReference &ref, bool forceTransfer = false); /**< @overload */

    /// Returns the asset provider that is used to fetch assets from the given full URL.
    /** Example: GetProviderForAssetRef("local://my.mesh") will return an instance of LocalAssetProvider.
        @param assetRef The asset reference name to query a provider for.
        @param assetType An optionally specified asset type. Some providers can only handle certain asset types. This parameter can be 
                        used to more completely specify the type. */
    AssetProviderPtr ProviderForAssetRef(String assetRef, String assetType = "") const;

    /// Creates a new empty unloaded asset of the given type and name.
    /** This function uses the Asset type factories to create an instance of the proper asset class.
        @param type The asset type of the asset to be created. A factory of this type must have been registered beforehand,
                    using the AssetAPI::RegisterAssetTypeFactory function.
        @param name Specifies the name to give to the new asset. This name must be unique in the system, or this call will fail.
                    Use GetAsset(name) to query if an asset with the given name exists, and the AssetAPI::GenerateUniqueAssetName 
                    to guarantee the creation of a unique asset name. */
    AssetPtr CreateNewAsset(String type, String name);

    /// Creates a new empty unloaded asset bundle of the given type and name.
    AssetBundlePtr CreateNewAssetBundle(String type, String name);

    /// Loads an asset from a local file.
    AssetPtr CreateAssetFromFile(String assetType, String assetFile);

    /// Generates a new asset name that is guaranteed to be unique in the system.
    /** @param assetTypePrefix The type of the asset to use as a human-readable visual prefix identifier for the name. May be empty.
        @param assetNamePrefix A name prefix that is added to the asset name for visual identification. May be empty.
        @return A string of the form "Asset_<assetTypePrefix>_<assetNamePrefix>_<number>". */
    String GenerateUniqueAssetName(String assetTypePrefix, String assetNamePrefix) const;

    /// Generates an absolute path name to a file on the local system that is guaranteed to be writable to and nonexisting.
    /** This file can be used as temporary workspace for asset serialization/deserialization routines. This is used especially with
        Ogre-related data (de)serializers, since they don't have support for loading/saving data from memory and need to access a file. */
    String GenerateTemporaryNonexistingAssetFilename(String filename) const;

    /// Returns the asset type factory that can create assets of the given type, or null, if no asset type provider of the given type exists.
    AssetTypeFactoryPtr AssetTypeFactory(const String &typeName) const;

    /// Return the asset bundle factory that can create asset bundles of the given type, or null, if no asset bundle type provider of the given type exists.
    AssetBundleTypeFactoryPtr AssetBundleTypeFactory(const String &typeName) const;

    /// Returns the given asset by full URL ref if it exists, or null otherwise.
    /// @note The "name" of an asset is in most cases the URL ref of the asset, so use this function to query an asset by name.
    AssetPtr FindAsset(String assetRef) const;

    /// Returns the given asset bundle by full URL ref if it exists, or null otherwise.
    /// @note The "name" of an asset is in most cases the URL ref of the asset bundle, so use this function to query an asset bundle by name.
    AssetBundlePtr FindBundle(String bundleRef) const;

    /// Returns the asset cache object that generates a disk source for all assets.
    AssetCache *Cache() const { return assetCache; }

    /// Returns the asset storage of the given name.
    /// @param name The name of the storage to get. Remember that Asset Storage names are case-insensitive.
    AssetStoragePtr AssetStorageByName(const String &name) const;

    /// Returns the asset storage for a given asset ref.
    /// @param ref The ref to search for.
    AssetStoragePtr StorageForAssetRef(const String& ref) const;

    /// Removes the given asset storage from the list of all asset storages.
    /// The scene can still refer to assets in this storage, and download requests can be performed to it, but it will not show up in the Assets dialog,
    /// and asset upload operations cannot be performed to it. Also, it will not be used as a default storage.
    /// @param name The name of the storage to get. Remember that Asset Storage names are case-insensitive.
    bool RemoveAssetStorage(const String &name);

    /// Creates an asset storage from the given serialized string form.
    /** Returns a null pointer if the given storage could not be added.
        @param fromNetwork If true, treats the storage specifier as if the storage had been received from the network, and not from the local computer.
        @code
        The string needs to be in the form of "param1=value1;param2=value2;" or a single string value to the storage source
        "http://my.storage.com/assets" that will be auto converted to "src=<value>".
        Following parameters are supported:
            src=[string];
            name=[string];
            recursive=[bool-as-string];
            readonly=[bool-as-string];
            liveupdate=[bool-as-string];
            autodiscoverable=[bool-as-string];
            replicated=[bool-as-string];
            trusted=[true|ask|untrusted];
        Valid boolean values are "true", "1", "false", "0". 
        All parameters may not apply to all storage implementations.
        @endcode */
    AssetStoragePtr DeserializeAssetStorageFromString(const String &storage, bool fromNetwork);

    /// Returns the AssetStorage that should be used by default when assets are requested by their local name only, e.g. when an assetRef only contains
    /// a string "texture.png" and nothing else.
    AssetStoragePtr DefaultAssetStorage() const;

    /// Sets the asset storage to be used when assets are requested by their local names.
    void SetDefaultAssetStorage(const AssetStoragePtr &storage);

    /// Tries to find the filename in an url/assetref.
    /** For example, all "my.mesh", "C:\files\my.mesh", "local://path/my.mesh", "http://www.web.com/my.mesh" will return "my.mesh".
        "local://collada.dae,subMeshName" returns "collada.dae". */
    static String ExtractFilenameFromAssetRef(String ref);

    /// Returns an asset type name of the given assetRef. e.g. "asset.png" -> "Texture".
    /** The Asset type name is a unique type identifier string each asset type has. */
    String ResourceTypeForAssetRef(String assetRef) const;
    String ResourceTypeForAssetRef(const AssetReference &ref) const; /**< @overload */

    /// Parses a (relative) assetRef in the given context, and returns an assetRef pointing to the same asset as an absolute asset ref.
    /** For example: context: "local://myasset.material", ref: "texture.png" returns "local://texture.png".
        context: "http://myserver.com/path/myasset.material", ref: "texture.png" returns "http://myserver.com/path/texture.png".
        The context string may be left empty, in which case the current default storage (DefaultAssetStorage) is used as the context.
        If ref is an absolute asset reference, it is returned unmodified (no need for context). */
    String ResolveAssetRef(String context, String ref) const;

    /// Given an assetRef, turns it into a native OS file path to the asset.
    /** The given ref is resolved in the context of "local://", if it is a relative asset ref.
        If ref contains a subAssetName, it is stripped from outFilePath, and returned in subAssetName.
        If the assetRef doesn't represent a file on the local filesystem, FileQueryExternalFile is returned and outFilePath is set to equal best effort to parse 'ref' locally. */
    FileQueryResult ResolveLocalAssetPath(String ref, String baseDirectoryContext, String &outFilePath, String *subAssetName = 0) const;

    /// Recursively iterates through the given path and all its subdirectories and tries to find the given file.
    /** Returns the absolute path for that file, if it exists. The path contains the filename,
        i.e. it is of form "C:\folder\file.ext" or "/home/username/file.ext". */
    static String RecursiveFindFile(String basePath, String filename);

    /// Removes the given asset from the system and frees up all resources related to it.
    /** Any assets depending on this asset will break.
        @param assetRef A valid assetRef that is in the asset system. If this asset ref does not exist, this call will do nothing.
        @param removeDiskSource If true, the disk source of the asset is also deleted. In most cases, this is the locally cached version of the remote file,
               but for example for local assets, this is the asset itself.
        @return True if asset was located and unloaded, false otherwise.
        @note Calling ForgetAsset on an asset will unload it from the system. Do not dereference the asset after calling this function. */
    bool ForgetAsset(AssetPtr asset, bool removeDiskSource);
    bool ForgetAsset(String assetRef, bool removeDiskSource); /**< @overload */

    /// Removes the given asset bundle from the system and frees up all resources related to it, except its sub assets.
    /** @param bundleRef A valid bundleRef that is in the asset system. If this asset ref does not exist, this call will do nothing.
        @param removeDiskSource If true, the disk source of the asset is also deleted. In most cases, this is the locally cached version of the remote file,
               but for example for local assets, this is the asset itself.
        @return True if asset bundle was located and unloaded, false otherwise.
        @note Calling ForgetBundle on an asset bundle will unload it from the system. Do not dereference the asset after calling this function. 
        @note Calling ForgetBundle will not unload any sub assets that might be loaded from this bundle. For this use ForgetAsset. */
    bool ForgetBundle(AssetBundlePtr bundle, bool removeDiskSource);
    bool ForgetBundle(String bundleRef, bool removeDiskSource); /**< @overload */

    /// Sends an asset deletion request to the remote asset storage the asset resides in.
    /// @note Calling DeleteAssetFromStorage on an asset will unload it from the system, *and* delete the disk source of this asset. Do not dereference 
    /// the asset after calling this function. After calling this function, the asset will be gone from everywhere!
    void DeleteAssetFromStorage(String assetRef);

    /// Uploads an asset to an asset storage.
    /** @param filename The source file to load the asset from.
        @param storageName The asset storage to upload the asset to.
        @param assetName Optional name to give to the asset in the storage. If this is not given assetName = original filename.
        @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process.
        
        @note This is a script friendly override. This function will never throw Exceptions (CoreException.h) if passed data is invalid, but will return null AssetUploadTransferPtr. */
    AssetUploadTransferPtr UploadAssetFromFile(const String &filename, const String &storageName, const String &assetName = "");

    /// Uploads an asset to an asset storage.
    /** @param filename The source file to load the asset from.
        @param destination The asset storage to upload the asset to.
        @param assetName The name to give to the asset in the storage.
        @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process.

        @note This function will never return 0, but instead will throw Exception (CoreException.h) if passed data is invalid. */
    AssetUploadTransferPtr UploadAssetFromFile(const String &filename, AssetStoragePtr destination, const String &assetName);

    /// Uploads an asset from the given data pointer in memory to an asset storage.
    /** @param data A pointer to raw source data in memory.
        @param numBytes The amount of data in the data array.
        @param destination The asset storage to upload the asset to.
        @param assetName The name to give to the asset in the storage.
        @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process, or null if passed data is invalid. */
    AssetUploadTransferPtr UploadAssetFromFileInMemory(const u8 *data, uint numBytes, AssetStoragePtr destination, const String &assetName);

    /// Unloads all known assets, and removes them from the list of internal assets known to the Asset API.
    /** Use this to clear the client's memory from all assets.
        @note There may be any number of strong references to assets in other parts of code, in which case the assets are not deleted
        until the refcounts drop to zero.
        @note Do not dereference any asset pointers that might have been left over after calling this function. */
    void ForgetAllAssets();

    /// Returns a pointer to an existing asset transfer if one is in-progress for the given assetRef.
    /** Returns a null pointer if no transfer exists, in which case the asset may already have been loaded to the system (or not).
        It can be that an asset is loaded to the system, but one or more of its dependencies have not, in which case there exists
        both an IAssetTransfer and IAsset to this particular assetRef (so the existence of these two objects is not mutually exclusive).
        @note Client code should not need to worry about whether a particular transfer is pending or not, but simply call RequestAsset whenever an asset
        request is needed. AssetAPI will optimize away any duplicate transfers to the same asset. */
    AssetTransferPtr PendingTransfer(String assetRef) const;

    /// Starts an asset transfer for each dependency the given asset has.
    void RequestAssetDependencies(AssetPtr transfer);

    /// A utility function that counts the number of dependencies the given asset has to other assets that have not been loaded in.
    int NumPendingDependencies(AssetPtr asset) const;

    /// A utility function that returns true if the given asset still has some unloaded dependencies left to process.
    /// @note For performance reasons, calling this function is highly advisable instead of calling NumPendingDependencies, if it is only
    ///       desirable to known whether the asset has any pending dependencies or not.
    bool HasPendingDependencies(AssetPtr asset) const;

    /// Handle discovery of a new asset through the AssetDiscovery network message
    void HandleAssetDiscovery(const String &assetRef, const String &assetType);

    /// Handle deletion of an asset through the AssetDeleted network message
    void HandleAssetDeleted(const String &assetRef);
    
    /// Cause AssetAPI to emit AssetDeletedFromStorage. Called by asset providers
    void EmitAssetDeletedFromStorage(const String &assetRef);
    
    /// Called by all providers to inform Asset API whenever they have added a new asset storage to the provider.
    void EmitAssetStorageAdded(AssetStoragePtr newStorage);

    /// Return current asset transfers
    AssetTransferMap CurrentTransfers() const { return currentTransfers; }

    /// A utility function that counts the number of current asset transfers.
    size_t NumCurrentTransfers() const { return currentTransfers.size(); }
    
    /// Return the current asset dependency map (debugging)
    const AssetDependenciesMap& DebugGetAssetDependencies() const { return assetDependencies; }
    
    /// Return ready asset transfers (debugging)
    const Vector<AssetTransferPtr>& DebugGetReadyTransfers() const { return readyTransfers; }

    /// Emitted for each new asset that was created and added to the system.
    /** When this signal is triggered, the dependencies of an asset may not yet have been loaded. */
    Signal1<AssetPtr> AssetCreated;

    /// Emitted before an asset is going to be forgotten.
    Signal1<AssetPtr> AssetAboutToBeRemoved;

    /// Emitted before an asset bundle is going to be forgotten.
    Signal1<AssetBundlePtr> AssetBundleAboutToBeRemoved;

    /// Emitted before an assets disk source will be removed.
    Signal1<AssetPtr> DiskSourceAboutToBeRemoved;

    /// Emitted before an asset bundles disk source will be removed.
    Signal1<AssetBundlePtr> BundleDiskSourceAboutToBeRemoved;

    /// An asset's disk source has been modified. Practically only emitted for files in the asset cache.
    Signal1<AssetPtr> AssetDiskSourceChanged;

    /// Emitted when an asset has been uploaded
    Signal1<const String&> AssetUploaded;

    /// Emitted when asset was confirmedly deleted from storage
    Signal1<const String&> AssetDeletedFromStorage;

    /// Emitted when an asset storage has been added
    Signal1<AssetStoragePtr> AssetStorageAdded;

    /// Emitted when the contents of an asset disk source has changed. ///\todo Implement.
    //void AssetDiskSourceChanged(AssetPtr asset);

    /// Emitted when the asset has changed in the remote AssetStorage it is in. ///\todo Implement.
    //void AssetStorageSourceChanged(AssetPtr asset);

private:
    /// The Asset API listens on each asset when they get loaded, to track the completion of the dependencies of other loaded assets.
    void OnAssetLoaded(AssetPtr asset);

    /// The Asset API reloads all assets from file when their disk source contents change.
    void OnAssetDiskSourceChanged(const String &path);

    /// Contents of asset storage has been changed.
    void OnAssetChanged(IAssetStorage* storage, String localName, String diskSource, IAssetStorage::ChangeType change);

    /// Listens to the IAssetBundle Loaded signal.
    void AssetBundleLoadCompleted(IAssetBundle *bundle);

    /// Listens to the IAssetBundle Failed signal.
    void AssetBundleLoadFailed(IAssetBundle *bundle);

private:
    AssetTransferMap::iterator FindTransferIterator(String assetRef);
    AssetTransferMap::const_iterator FindTransferIterator(String assetRef) const;

    AssetTransferMap::iterator FindTransferIterator(IAssetTransfer *transfer);
    AssetTransferMap::const_iterator FindTransferIterator(IAssetTransfer *transfer) const;

    /// Cleans up everything in the Asset API.
    /** Forgets all assets, kills all asset transfers, frees all storages, providers, and type factories.
        Deletes the asset cache and the disk watcher. Called by Framework. */
    void Reset();

    /// Removes from AssetDependenciesMap all dependencies the given asset has.
    void RemoveAssetDependencies(String asset);

    /// Handle discovery of a new asset, when the storage is already known. This is used internally for optimization, so that providers don't need to be queried
    void HandleAssetDiscovery(const String &assetRef, const String &assetType, AssetStoragePtr storage);
    
    /// Create new asset, when the storage is already known. This is used internally for optimization
    AssetPtr CreateNewAsset(String type, String name, AssetStoragePtr storage);

    /// Load sub asset to transfer. Used internally for loading sub asset from bundle to virtual transfers.
    bool LoadSubAssetToTransfer(AssetTransferPtr transfer, const String &bundleRef, const String &fullSubAssetRef, String subAssetType = String());

    /// Overload that takes in AssetBundlePtr instead of refs.
    bool LoadSubAssetToTransfer(AssetTransferPtr transfer, IAssetBundle *bundle, const String &fullSubAssetRef, String subAssetType = String());

    bool isHeadless;

    /// Stores all the currently ongoing asset transfers.
    AssetTransferMap currentTransfers;

    /// Stores all currently pending transfers.
    AssetTransferPtrVector pendingTransfers_;

    /// Asset transfer prioritizer.
    AssetTransferPrioritizerPtr transferPrioritizer_;

    /// Stores all the currently ongoing asset bundle monitors.
    AssetBundleMonitorMap bundleMonitors;

    typedef std::map<String, AssetUploadTransferPtr, StringCompareCaseInsensitive> AssetUploadTransferMap;
    /// Stores all the currently ongoing asset uploads, maps full assetRefs to the asset upload transfer structures.
    AssetUploadTransferMap currentUploadTransfers;

    /// Keeps track of all the dependencies each asset has to each other asset.
    /// \todo Find a more effective data structure for this. Needs something like boost::bimap but for multi-indices.
    AssetDependenciesMap assetDependencies;

    /// Stores a list of asset requests to assets that have already been downloaded into the system. These requests don't go to the asset providers
    /// to process, but are internally filled by the Asset API. This member vector is needed to be able to delay the requests and virtual completions
    /// by one frame, so that the client gets a chance to connect his handler's signals to the AssetTransferPtr slots.
    Vector<AssetTransferPtr> readyTransfers;
    
    // Stores a list of sub asset requests that are pending a load from a loaded asset bundle.
    Vector<SubAssetLoader> readySubTransfers;

    /// Contains all known asset storages in the system.
    //Vector<AssetStoragePtr> storages;

    /// Specifies the storage to use for asset requests with local name only.
    AssetStorageWeakPtr defaultStorage;

    /// Stores all the registered asset type factories in the system.
    Vector<AssetTypeFactoryPtr> assetTypeFactories;

    /// Stores all the registered asset bundle type factories in the system.
    Vector<AssetBundleTypeFactoryPtr> assetBundleTypeFactories;

    /// Stores a list of asset requests that the Asset API hasn't started at all but has put on hold, until other operations complete.
    /// This data structure is used to enforce that asset uploads are completed before any asset downloads to that asset.
    struct PendingDownloadRequest
    {
        String assetRef;
        String assetType;
        AssetTransferPtr transfer;
    };
    typedef std::map<String, PendingDownloadRequest, StringCompareCaseInsensitive> PendingDownloadRequestMap;
    PendingDownloadRequestMap pendingDownloadRequests;

    /// Stores all the already loaded assets in the system.
    AssetMap assets;

    /// Stores all the already loaded asset bundles in the system.
    AssetBundleMap assetBundles;

    /// Specifies all the registered asset providers in the system.
    Vector<AssetProviderPtr> providers;

    Framework *fw;
    SharedPtr<AssetCache> assetCache;
};

}

#include "AssetAPI.inl"
