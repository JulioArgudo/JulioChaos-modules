#include "plugin.hpp"

struct JulioChua : Module {
    float sr;
    float x=0.1;
    float y=0.f;
    float z=0.f;
    bool range=false;
    
    float rateScale = 800.0;
    float preDCx=0.f;
    float postDCx=0.f;
    float preDCy=0.f;
    float postDCy=0.f;
    float preDCz=0.f;
    float postDCz=0.f;

    float rate;         // 0 a 800 /sr
    float C1=15.6;      // 15.6 a 15.7
    float C2=1.f;       // 0.98 a 1.9
    float C3=27.9;      // 27.6 a 28.5
    float m0=-1.143;    // -1.143 a -1.23
    float m1=-0.754;    // -0.754 a -0.8
    unsigned int oversampling=6;

    enum ParamId {
        RATE_PARAM,
        P1_PARAM,
        P2_PARAM,
        P3_PARAM,
        CVRATE_PARAM,
        CVP1_PARAM,
        CVP2_PARAM,
        CVP3_PARAM,
        RANGE_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        INRATE_INPUT,
        INP1_INPUT,
        INP2_INPUT,
        INP3_INPUT,
        INRESET_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        X_OUTPUT,
        Y_OUTPUT,
        Z_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        RANGE_LIGHT,
        LIGHTS_LEN
    };
    
    dsp::ClockDivider lightDivider;
    dsp::SchmittTrigger resetTrigger;

    JulioChua() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(RATE_PARAM, 0.f, 1.f, 0.f, "Rate");
        configParam(P1_PARAM, 0.f, 1.f, 0.5f, "P1");
        configParam(P2_PARAM, 0.f, 1.f, 0.5f, "P2");
        configParam(P3_PARAM, 0.f, 1.f, 0.5f, "P3");
        configParam(CVRATE_PARAM, -1.f, 1.f, 0.f, "Rate CV");
        configParam(CVP1_PARAM, -1.f, 1.f, 0.f, "P1 CV");
        configParam(CVP2_PARAM, -1.f, 1.f, 0.f, "P2 CV");
        configParam(CVP3_PARAM, -1.f, 1.f, 0.f, "P3 CV");
        configSwitch(RANGE_PARAM, 0.f, 1.f, 0.f, "Fq Range", {"HF", "LF"});
        configInput(INRATE_INPUT, "Rate");
        configInput(INP1_INPUT, "P1");
        configInput(INP2_INPUT, "P2");
        configInput(INP3_INPUT, "P3");
        configInput(INRESET_INPUT, "Reset");
        configOutput(X_OUTPUT, "X");
        configOutput(Y_OUTPUT, "Y");
        configOutput(Z_OUTPUT, "Z");
        
        lightDivider.setDivision(16);
    }

    void process(const ProcessArgs& args) override {
        // Frequency range
        bool range = params[RANGE_PARAM].getValue() > 0.f;
        if (range) {
            oversampling=1;
            rateScale = 100.f;
        }
        else {
            oversampling=6;
            rateScale = 800.f;
        }
                
        //Parameters
        sr=args.sampleRate;
        rate=params[RATE_PARAM].getValue()*rateScale / sr;      // rate 0 a 800 /sr
        C1=params[P3_PARAM].getValue()*0.1 + 15.6;              // C1 15.6 a 15.7
        C2=params[P1_PARAM].getValue()*1.1 + 0.98;              // C2 0.98 a 1.9
        C3=params[P3_PARAM].getValue()*0.6 + 27.9;              // C3 27.6 a 28.5
        m0=params[P2_PARAM].getValue()*-0.087 - 1.143;          // m0 -1.143 a -1.23
        m1=params[P3_PARAM].getValue()*-0.046 - 0.754;          // m1 -0.754 a -0.8
        
        // CV
        float CVrate = inputs[INRATE_INPUT].getVoltage() * params[CVRATE_PARAM].getValue() * rateScale*0.037/sr;
        rate = clamp(rate + CVrate, 0.f, rateScale/sr);
        float CVC1 = inputs[INP3_INPUT].getVoltage() * params[CVP3_PARAM].getValue() * 0.04;
        C1 = clamp(C1 + CVC1, 15.6f, 15.7f);
        float CVC2 = inputs[INP1_INPUT].getVoltage() * params[CVP1_PARAM].getValue() * 0.5;
        C2 = clamp(C2 + CVC2, 0.98f, 1.9f);
        float CVC3 = inputs[INP3_INPUT].getVoltage() * params[CVP3_PARAM].getValue() * 0.25;
        C3 = clamp(C3 + CVC3, 27.6f, 28.5f);
        float CVm0 = inputs[INP2_INPUT].getVoltage() * params[CVP2_PARAM].getValue() * -0.04;
        m0 = clamp(m0 + CVm0, -1.23f, -1.143f);
        float CVm1 = inputs[INP3_INPUT].getVoltage() * params[CVP3_PARAM].getValue() * -0.02;
        m1 = clamp(m1 + CVm1, -0.8f, -0.754f);
        
        // Chua solution by Euler
        for(unsigned int i=0; i<oversampling; i++) {
            float h = x*m1+(m0-m1)*0.5*(std::abs(x+1)-std::abs(x-1));
            float dx = C1*(y-x-h);
            float dy = C2*(x-y+z);
            float dz = -C3*y;
            x = x+rate*dx;
            y = y+rate*dy;
            z = z+rate*dz;
        }

        // Reset
        bool clip=false;
        if (std::abs(x)>12.f || std::abs(y)>12.f || std::abs(z)>12.f) {clip=true;}
        float resetSignal = inputs[INRESET_INPUT].getVoltage();
        float resetBang = resetTrigger.process(resetSignal, 0.1f, 2.f);
        if (resetBang || clip) {
            x=0.1;
            y=0;
            z=0;
            clip=false;
        }

        // DC Blocker
        postDCx = x - preDCx + 0.9999*postDCx;
        preDCx = x;
        postDCy = y - preDCy + 0.9999*postDCy;
        preDCy = y;
        postDCz = z - preDCz + 0.9999*postDCz;
        preDCz = z;

        // Outputs
        outputs[X_OUTPUT].setVoltage(2.5f * postDCx);
        outputs[Y_OUTPUT].setVoltage(8.5f * postDCy);
        outputs[Z_OUTPUT].setVoltage(1.8f * postDCz);
        
        // Lights
        if (lightDivider.process()) {
            lights[RANGE_LIGHT].setBrightness(range);
        }
    }
};

struct JulioChuaWidget : ModuleWidget {
    JulioChuaWidget(JulioChua* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/JulioChua.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(12.566, 26.602)), module, JulioChua::RATE_PARAM));
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(32.821, 26.602)), module, JulioChua::P1_PARAM));
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(12.566, 56.017)), module, JulioChua::P2_PARAM));
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(32.821, 56.017)), module, JulioChua::P3_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(6.656, 80.59)), module, JulioChua::CVRATE_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(17.443, 80.59)), module, JulioChua::CVP1_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(28.293, 80.59)), module, JulioChua::CVP2_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(39.061, 80.59)), module, JulioChua::CVP3_PARAM));
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(22.694, 42.309)), module, JulioChua::RANGE_PARAM, JulioChua::RANGE_LIGHT));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.452, 97.459)), module, JulioChua::INRATE_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.089, 97.459)), module, JulioChua::INP1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28.424, 97.459)), module, JulioChua::INP2_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.061, 97.459)), module, JulioChua::INP3_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.452, 113.438)), module, JulioChua::INRESET_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(17.089, 113.438)), module, JulioChua::X_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(28.424, 113.438)), module, JulioChua::Y_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.061, 113.438)), module, JulioChua::Z_OUTPUT));
    }
};

Model* modelJulioChua = createModel<JulioChua, JulioChuaWidget>("JulioChua");
