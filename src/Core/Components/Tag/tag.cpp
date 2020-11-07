#include "tag.h"

Engine::TagComponent::TagComponent(const char* tag) : m_tag{tag} {}

Engine::TagComponent::TagComponent(std::string &tag) : m_tag{tag} {}

const std::string& Engine::TagComponent::get(){
    return m_tag;
}

void Engine::TagComponent::set(const char* tag) {
    m_tag = tag;
}

void Engine::TagComponent::set(std::string &tag) {
    m_tag = tag;
}