#pragma once

#include <openbbq/control/ControlPID.h>
#include <display/ui/Layout.h>
#include <display/ui/Label.h>

namespace bbq::ui
{

    class ControlSignalLabel : public Label
    {
    public:
        ControlSignalLabel(const StylePtr &style, const ControlSignal<float> &signal, int decimalPlaces, const char *suffix)
            : Label(style), signal(signal), decimalPlaces(decimalPlaces), suffix(suffix)
        {
        }

        static std::shared_ptr<ControlSignalLabel> create(const StylePtr &style, const ControlSignal<float> &signal, int decimalPlaces, const char *suffix)
        {
            auto ptr = std::make_shared<ControlSignalLabel>(style, signal, decimalPlaces, suffix);
            ptr->alignment(DrawContext::RIGHT);
            return ptr;
        }

        void loopHandler() override
        {
            Label::loopHandler();
            text(String(signal.value(), decimalPlaces) + suffix);
        }

        const ControlSignal<float> &signal;
        int decimalPlaces;
        const char *suffix;
    };

    class TuneView : public Background
    {
    public:
        struct ViewModel
        {
            ControlPID &pid;
        };

        TuneView(const StylePtr &style, const ViewModel &vm) : Background(style), model(vm){};

        static std::shared_ptr<TuneView> create(const StylePtr &style, const ViewModel &vm)
        {
            auto ptr = std::make_shared<TuneView>(style, vm);

            ptr->add("Setpoint", vm.pid._setpoint.output, 1, " F");
            ptr->add("Process Value", vm.pid._processValue, 1, " F");
            ptr->add("Output", vm.pid._out, 1, "%");
            ptr->addTop(Background::create(style), 4);
            ptr->add("Band", vm.pid._error, 1, " F", &vm.pid._band, 2);
            ptr->add("Proportional", vm.pid._proportional, 1, "%", &vm.pid._Kp, 2);
            ptr->add("Integral", vm.pid._integral, 1, "%", &vm.pid._Ki, 2);
            ptr->add("Derivative", vm.pid._derivative, 1, "%", &vm.pid._Kd, 2);

            return ptr;
        }

        void add(const char *text,
                 const ControlSignal<float> &signal,
                 int decimalPlaces,
                 const char *suffix,
                 const ControlSignal<float> *prefix = nullptr,
                 int prefixPlaces = 0)
        {
            auto f = style()->font();
            auto bar = std::make_shared<Layout>();
            bar->addRight(ControlSignalLabel::create(style(), signal, decimalPlaces, suffix),
                          80);
            if (prefix)
            {
                bar->addLeft(ControlSignalLabel::create(style(), *prefix, prefixPlaces, " "),
                             f->measure("0.00 ").width());
            }
            bar->addFill(Label::create(style(), text, DrawContext::LEFT));
            // TODO(loudej) #13 flex layout should measure children automatically
            addTop(bar, f->measure("0").height());
        }

        ViewModel model;
    };
} // namespace bbq::ui
