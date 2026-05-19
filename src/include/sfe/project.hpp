#pragma once
#include<bbe/project_entity_pool.hpp>
#include"style.hpp"
namespace sfe{
    class Project{
        bbe::ProjectEntitiesPool decl;
        NameDatabase name;
        public:
            Project() = default;
            const bbe::ProjectEntitiesPool& entities() const{
                return decl;
            }
            bbe::ProjectEntitiesPool& entities(){
                return decl;
            }
            const NameDatabase& names() const{
                return name;
            }
            NameDatabase& names(){
                return name;
            }
    };
}
