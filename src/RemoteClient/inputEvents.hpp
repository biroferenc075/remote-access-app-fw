#pragma once
#include "consts.hpp"
#include <memory>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
namespace RAC {
    struct InputEvent {
        virtual unsigned char getTypeId() = 0;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version);

        template<class Archive>
        static InputEvent* load(Archive& ar);
    };
    struct MousePressEvent : public InputEvent {
        double xpos;
        double ypos;
        int button;
        int action;
        int mods;

        friend class boost::serialization::access;
        unsigned char getTypeId() { return 0; }

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar& getTypeId();
            ar& xpos;
            ar& ypos;
            ar& button;
            ar& action;
            ar& mods;
        }
        MousePressEvent(double xpos, double ypos, int button, int action, int mods) : xpos(xpos), ypos(ypos), button(button), action(action), mods(mods) {}
        template<class Archive>
        MousePressEvent(Archive& ar) {
            ar >> xpos;
            ar >> ypos;
            ar >> button;
            ar >> action;
            ar >> mods;
        }
    };

    struct MousePollEvent : public InputEvent {

        double xpos;
        double ypos;

        friend class boost::serialization::access;
        unsigned char getTypeId() { return 1; }
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar& getTypeId();
            ar& xpos;
            ar& ypos;

        }
        MousePollEvent(double xpos, double ypos) : xpos(xpos), ypos(ypos) {}

        template<class Archive>
        MousePollEvent(Archive& ar) {
            ar >> xpos;
            ar >> ypos;
        }
    };

    struct KeyboardEvent : public InputEvent {

        int key, scancode, action, mods;

        friend class boost::serialization::access;
        unsigned char getTypeId() { return 2; }
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar << getTypeId() << key << scancode << action << mods;
        }
        KeyboardEvent(int key, int scancode, int action, int mods) : key(key), scancode(scancode), action(action), mods(mods) {}
        template<class Archive>
        KeyboardEvent(Archive& ar) {
            ar >> key;
            ar >> scancode;
            ar >> action;
            ar >> mods;
        }
    };

    struct WindowResizeEvent : public InputEvent {
        int width, height;

        friend class boost::serialization::access;
        unsigned char getTypeId() { return 3; }
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar& getTypeId();
            ar& width;
            ar& height;
        }
        WindowResizeEvent(int width, int height) : width(width), height(height) {}
        template<class Archive>
        WindowResizeEvent(Archive& ar) {
            ar >> width;
            ar >> height;
        }
    };

    template<class Archive>
    InputEvent* InputEvent::load(Archive& ar) {
        unsigned char typeId;
        ar >> typeId;

        switch (typeId) {
        case 0:
            return new MousePressEvent(ar);
        case 1:
            return new MousePollEvent(ar);
        case 2:
            return new KeyboardEvent(ar);
        case 3:
            return new WindowResizeEvent(ar);
        }
    }
}