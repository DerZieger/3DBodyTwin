#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_CONSTRAINTS_H
#define TWIN_CONSTRAINTS_H

#include "twinDefines.h"
#include "net/constraints/constraint.h"

TWIN_NAMESPACE_BEGIN


    class ConstraintsImpl {
    public:
        using Change = ConstraintImpl::Change;

        void Clear();

        void Add(Constraint con);

        void Remove(const std::string &name);

        void Remove(long unsigned int index);

        std::lock_guard<std::recursive_mutex> Lock();

        std::vector<Constraint> &All();

        const std::vector<Constraint> &All() const;

        void render() const;

        void Update(Change change);

        void Updated();

        Change GetChange();

        bool NeedsUpdate();

        explicit ConstraintsImpl(const std::string& na);

        std::string name;
    private:
        std::vector<Constraint> m_constraints;

        Change m_change = Change::None;

        std::recursive_mutex m_lock;

        static int c_counter;

        int m_id;
    };

    using Contraints = cppgl::NamedHandle<ConstraintsImpl>;

TWIN_NAMESPACE_END
#endif //TWIN_CONSTRAINTS_H
#endif