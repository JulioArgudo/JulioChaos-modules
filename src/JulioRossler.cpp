#include "plugin.hpp"

struct JulioRossler : Module {
    float sr;
    float x=0.1;
    float y=0.3;
    float z=0.9;
    bool range=false;
    
    float rateScale = 600.0;
    float preDCx=0.f;
    float postDCx=0.f;
    float preDCy=0.f;
    float postDCy=0.f;
    float preDCz=0.f;
    float postDCz=0.f;
    
    float rate;         // 0 a 600 /sr
    float A=0.1f;
    float B=0.15f;
    float C=5.7f;
    unsigned int oversampling=6;
    
	enum ParamId {
		RATE_PARAM,
		A_PARAM,
		B_PARAM,
		C_PARAM,
		CVRATE_PARAM,
		CVA_PARAM,
		CVB_PARAM,
		CVC_PARAM,
        RANGE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INRATE_INPUT,
		INA_INPUT,
		INB_INPUT,
		INC_INPUT,
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

	JulioRossler() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RATE_PARAM, 0.f, 1.f, 0.f, "Rate");
		configParam(A_PARAM, 0.f, 1.f, 0.5f, "A");
		configParam(B_PARAM, 0.f, 1.f, 0.5f, "B");
		configParam(C_PARAM, 0.f, 1.f, 0.5f, "C");
		configParam(CVRATE_PARAM, -1.f, 1.f, 0.f, "Rate CV");
		configParam(CVA_PARAM, -1.f, 1.f, 0.f, "A CV");
		configParam(CVB_PARAM, -1.f, 1.f, 0.f, "B CV");
		configParam(CVC_PARAM, -1.f, 1.f, 0.f, "C CV");
        configSwitch(RANGE_PARAM, 0.f, 1.f, 0.f, "Fq Range", {"HF", "LF"});
		configInput(INRATE_INPUT, "Rate");
		configInput(INA_INPUT, "A");
		configInput(INB_INPUT, "B");
		configInput(INC_INPUT, "C");
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
            rateScale = 80.f;
        }
        else {
            oversampling=7;
            rateScale = 600.f;
        }
        
        //Parameters
        sr=args.sampleRate;
        rate=params[RATE_PARAM].getValue()*rateScale / sr;      // rate 0 a 600 /sr
        A=params[A_PARAM].getValue()*0.28f + 0.05f;              // A 0.05 a 0.33
        B=params[B_PARAM].getValue()*(-0.75f) + 0.9f;              // B 0.15 a 0.9
        C=params[C_PARAM].getValue()*11.f + 5.5f;              // C 5.5 a 16.5
        
        // CV
        float CVrate = inputs[INRATE_INPUT].getVoltage() * params[CVRATE_PARAM].getValue() * rateScale*0.037/sr;
        rate = clamp(rate + CVrate, 0.f, rateScale/sr);
        float CVA = inputs[INA_INPUT].getVoltage() * params[CVA_PARAM].getValue() * 0.12;
        A = clamp(A + CVA, 0.05f, 0.33f);
        float CVB = inputs[INB_INPUT].getVoltage() * params[CVB_PARAM].getValue() * -0.34;
        B = clamp(B + CVB, 0.15f, 0.9f);
        float CVC = inputs[INC_INPUT].getVoltage() * params[CVC_PARAM].getValue() * 5.;
        C = clamp(C + CVC, 5.5f, 16.5f);

        // Rossler solution by Euler
        for(unsigned int i=0; i<oversampling; i++) {
            float dx = -y-z;
            float dy = x+A*y;
            float dz = B+z*(x-C);
            x = x+rate*dx;
            y = y+rate*dy;
            z = z+rate*dz;
        }
        
        // Reset
        bool clip=false;
        if (std::abs(x*0.19f)>12.f || std::abs(y*0.19f)>12.f || std::abs(z*0.019f)>12.f) {clip=true;}
        float resetSignal = inputs[INRESET_INPUT].getVoltage();
        float resetBang = resetTrigger.process(resetSignal, 0.1f, 2.f);
        if (resetBang || clip) {
            x=0.1;
            y=0.3;
            z=0.9;
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
        outputs[X_OUTPUT].setVoltage(0.2f * postDCx);
        outputs[Y_OUTPUT].setVoltage(0.2f * postDCy);
        outputs[Z_OUTPUT].setVoltage(0.03f * postDCz);
        
        // Lights
        if (lightDivider.process()) {
            lights[RANGE_LIGHT].setBrightness(range);
        }
	}
};


struct JulioRosslerWidget : ModuleWidget {
	JulioRosslerWidget(JulioRossler* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/JulioRossler.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(12.566, 26.602)), module, JulioRossler::RATE_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(32.821, 26.602)), module, JulioRossler::A_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(12.566, 56.017)), module, JulioRossler::B_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(32.821, 56.017)), module, JulioRossler::C_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(6.656, 80.59)), module, JulioRossler::CVRATE_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(17.443, 80.59)), module, JulioRossler::CVA_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(28.293, 80.59)), module, JulioRossler::CVB_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(39.061, 80.59)), module, JulioRossler::CVC_PARAM));
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(22.694, 42.309)), module, JulioRossler::RANGE_PARAM, JulioRossler::RANGE_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.452, 97.459)), module, JulioRossler::INRATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.089, 97.459)), module, JulioRossler::INA_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28.424, 97.459)), module, JulioRossler::INB_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.061, 97.459)), module, JulioRossler::INC_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.452, 113.438)), module, JulioRossler::INRESET_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(17.089, 113.438)), module, JulioRossler::X_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(28.424, 113.438)), module, JulioRossler::Y_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.061, 113.438)), module, JulioRossler::Z_OUTPUT));
	}
};


Model* modelJulioRossler = createModel<JulioRossler, JulioRosslerWidget>("JulioRossler");
