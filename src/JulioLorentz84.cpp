#include "plugin.hpp"


struct JulioLorentz84 : Module {
    float sr;
    float x=0.1;
    float y=0.1;
    float z=0.1;
    bool range=false;
    
    float rateScale = 800.0;
    float preDCx=0.f;
    float postDCx=0.f;
    float preDCy=0.f;
    float postDCy=0.f;
    float preDCz=0.f;
    float postDCz=0.f;
    
    float rate;         // 0 a 800 /sr
    float a=0.25f;
    float b=4.f;
    float F=8.f;
    float G=1.25f;
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
    
	JulioLorentz84() {
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
        b=params[P1_PARAM].getValue()*2.4f + 2.6f;              // b 2.6 a 5
        F=params[P2_PARAM].getValue()*2.6f + 7.4f;              // F 7.3 a 10
        G=params[P3_PARAM].getValue()*-0.6f + 1.3f;              // G 1.3 a 0.7
	
        // CV
        float CVrate = inputs[INRATE_INPUT].getVoltage() * params[CVRATE_PARAM].getValue() * rateScale*0.037f/sr;
        rate = clamp(rate + CVrate, 0.f, rateScale/sr);
        float CVb = inputs[INP1_INPUT].getVoltage() * params[CVP1_PARAM].getValue() * 1.1f;
        b = clamp(b + CVb, 2.6f, 5.f);
        float CVF = inputs[INP2_INPUT].getVoltage() * params[CVP2_PARAM].getValue() * 1.2f;
        F = clamp(F + CVF, 7.4f, 10.f);
        float CVG = inputs[INP3_INPUT].getVoltage() * params[CVP3_PARAM].getValue() * -0.2f;
        G = clamp(G + CVG, 0.7f, 1.3f);
    
        // Lorentz84 solution by Euler
        for(unsigned int i=0; i<oversampling; i++) {
            float dx = -y*y-z*z-a*x+a*F;
            float dy = x*y-b*x*z-y+G;
            float dz = b*x*y+x*z-z;
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
            y=0.1;
            z=0.1;
            clip=false;
        }
        
        // DC Blocker
        postDCx = x - preDCx + 0.9999*postDCx;
        preDCx = x;
        postDCy = y - preDCy + 0.9999*postDCy;
        preDCy = y;
        postDCz = z - preDCz + 0.9999*postDCz;
        preDCz = z;

        // Outputs (cruzados)
        outputs[Y_OUTPUT].setVoltage(2.5f * postDCx);
        outputs[Z_OUTPUT].setVoltage(2.5f * postDCy);
        outputs[X_OUTPUT].setVoltage(2.5f * postDCz);
        
        // Lights
        if (lightDivider.process()) {
            lights[RANGE_LIGHT].setBrightness(range);
        }
    }
};


struct JulioLorentz84Widget : ModuleWidget {
	JulioLorentz84Widget(JulioLorentz84* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/JulioLorentz84.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(12.566, 26.602)), module, JulioLorentz84::RATE_PARAM));
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(32.821, 26.602)), module, JulioLorentz84::P1_PARAM));
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(12.566, 56.017)), module, JulioLorentz84::P2_PARAM));
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(32.821, 56.017)), module, JulioLorentz84::P3_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(6.656, 80.59)), module, JulioLorentz84::CVRATE_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(17.443, 80.59)), module, JulioLorentz84::CVP1_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(28.293, 80.59)), module, JulioLorentz84::CVP2_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(39.061, 80.59)), module, JulioLorentz84::CVP3_PARAM));
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(22.694, 42.309)), module, JulioLorentz84::RANGE_PARAM, JulioLorentz84::RANGE_LIGHT));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.452, 97.459)), module, JulioLorentz84::INRATE_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.089, 97.459)), module, JulioLorentz84::INP1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28.424, 97.459)), module, JulioLorentz84::INP2_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.061, 97.459)), module, JulioLorentz84::INP3_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.452, 113.438)), module, JulioLorentz84::INRESET_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(17.089, 113.438)), module, JulioLorentz84::X_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(28.424, 113.438)), module, JulioLorentz84::Y_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.061, 113.438)), module, JulioLorentz84::Z_OUTPUT));
	}
};


Model* modelJulioLorentz84 = createModel<JulioLorentz84, JulioLorentz84Widget>("JulioLorentz84");
