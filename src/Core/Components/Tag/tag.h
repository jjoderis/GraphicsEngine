#ifndef CORE_COMPONENTS_TAG
#define CORE_COMPONENTS_TAG

#include <string>

namespace Engine {
    class TagComponent {
    private:
        std::string m_tag;
    public: 
        TagComponent() = delete;

        TagComponent(const char* tag);
        TagComponent(std::string &tag);

        const std::string& get();

        void set(const char* tag);
        void set(std::string &tag);
    };
}

#endif