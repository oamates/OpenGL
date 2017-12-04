#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <cstdint>

#include "dg_map_p.hpp"
#include "singleton.hpp"
#include "utility.hpp"

namespace Dg
{
    typedef uint32_t RKey;
    RKey const RKey_INVALID = 0;

    // base struct class for all resources
    struct Resource
    {
        Resource(RKey a_key) : m_key(a_key) {}

        virtual ~Resource() = 0;

        virtual bool IsInitialised() = 0;

        
        virtual Dg_Result Init() = 0;       //! The key is used to init the object.
        virtual Dg_Result DeInit() = 0;

        RKey GetKey() const { return m_key; }

        RKey m_key;
    };

    // resource handle 
    struct hResource
    {
        hResource() : m_resource(nullptr) {}
        ~hResource();
        hResource(hResource const & a_other);
        hResource & operator = (hResource const & a_other);

        Resource* operator -> ();       // Conversion operator
        Resource& operator * ();        // Conversion operator

        Resource* m_resource;
    };
    
    enum class rOption      // Options for individual resources
    {
        DEFAULT = 0,
        AutoInit = 1,       // Initialise the resource on registration
        AutoDeinit = 2      // Deinitialise a resource once number of registered users equals 0.
    };

    enum class rmOption     // ResourceManger options
    {
        DEFAULT = 0,
    };

    // General resource manager. Usage:
    // 1) Register all resources with RegisterResource()
    // 2) Request Resources with GetResource()
    struct ResourceManager : public Singleton<ResourceManager>
    {
        void SetOptions(uint32_t);                          // Set an option.
        bool CheckOption(rmOption);                         // Check an option.

        // Register a new resource:
        //   a_key: Associate a unique key with this resource
        //   a_file: Associate a file with this resource
        //   a_options: Options for this resource
        template<typename ResourceType> Dg_Result RegisterResource(RKey a_key, uint32_t a_options);

        
        Dg_Result GetResourceHandle(RKey, hResource &);     // Get a pointer to a resource. Will fail if the resouce has not been successfully registed first with RegisterResource().
        Dg_Result InitResource(RKey);                       // Initialise a particular resource. 
        Dg_Result InitAll();                                // Initialises all resources.
    
        void DeinitResource(RKey, bool a_force = false);    // Deinitialise a particular resource. 
        void DeinitAll(bool a_force = false);               // Deinitialises all resources.

        ResourceManager() : m_options(static_cast<uint32_t>(rmOption::DEFAULT)){}
        ~ResourceManager();

        void DeregisterUser(RKey);                          // Only the hResource class should be calling this function.
        Resource* RegisterUser(RKey);                       // Will initialise resource if not initialised. Only the hResource class should be calling this function.

        struct ResourceContainer
        {
            Resource* m_resource;
            unsigned m_nUsers;
            uint32_t m_opts;
        };

        uint32_t m_options;
        Dg::map_p<RKey, ResourceContainer> m_resourceList;
    };


    //--------------------------------------------------------------------------------
    //    @   ResourceManager::RegisterResource()
    //--------------------------------------------------------------------------------
    template <typename ResourceType> Dg_Result ResourceManager::RegisterResource(RKey a_key, uint32_t a_options)
    {
        if (a_key != RKey_INVALID)
            return DgR_Failure;

        int index = 0;
        if (m_resourceList.find(a_key, index))
            return DgR_Duplicate;

        ResourceContainer rc;
        rc.m_nUsers = 0;
        rc.m_opts = a_options;
        rc.m_resource = new ResourceType(a_key);

        if (a_options & static_cast<uint32_t>(rOption::AutoInit))
        {
            if (rc.m_resource->Init() != DgR_Success)
                return DgR_Failure;
        }

        m_resourceList.insert(a_key, rc);
        return DgR_Success;
    }
}

#endif
