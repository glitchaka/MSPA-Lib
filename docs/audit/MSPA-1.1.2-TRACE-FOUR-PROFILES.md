# MSPA 1.1.2 — Traza completa de cuatro perfiles

Código auditado: `9c0a8f740fdac51dba886a19e2c0456d1349d95e`.

El escenario usa el mismo estímulo para cuatro perfiles psicológicos definidos por el host y registra personalidad, `OBSERVE`, appraisal, creencias observadas e inferidas, emociones (intensidad/baseline/exceso episódico), mood, candidatos, consideraciones y selección.

```text
13: === PROFILE cautious agent=1 ===
13: STIMULUS predicate=1 positive=1 event_time=5 process_time=10
13: PERSONALITY persistence=0.7500 curiosity=0.3500 stability=0.4500 boldness=0.2000 sociability=0.4000
13: OBSERVE changed=1 urgency=3 decision_recommended=1
13: TRACE stage=observe detail=proposition processed weight=0.9329
13: TRACE stage=appraisal.consequence detail= weight=-0.6500
13: TRACE stage=appraisal.control detail= weight=0.4425
13: BELIEF predicate=1 positive=1 certainty=0.6014 source=0
13: BELIEF predicate=2 positive=1 certainty=0.5212 source=2
13: EMOTION name=joy intensity=0.0460 baseline=0.0460 excess=0.0000
13: EMOTION name=sadness intensity=0.3574 baseline=0.0297 excess=0.3278
13: EMOTION name=fear intensity=0.3723 baseline=0.0518 excess=0.3206
13: EMOTION name=anger intensity=0.0338 baseline=0.0338 excess=0.0000
13: EMOTION name=anxiety intensity=0.2696 baseline=0.0480 excess=0.2216
13: EMOTION name=aversion intensity=0.0310 baseline=0.0310 excess=0.0000
13: MOOD valence=-0.0136 activation=0.0148
13: CANDIDATE action=1 utility=0.1381
13:   CONSIDERATION name=cost raw=0.0000 weighted=-0.0000
13:   CONSIDERATION name=risk raw=0.4500 weighted=-0.5239
13:   CONSIDERATION name=progress raw=0.8000 weighted=0.6800
13:   CONSIDERATION name=novelty raw=0.4000 weighted=-0.0180
13:   CONSIDERATION name=social raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=learned_success raw=0.5000 weighted=0.0000
13: CANDIDATE action=2 utility=-0.0031
13:   CONSIDERATION name=cost raw=0.0800 weighted=-0.0800
13:   CONSIDERATION name=risk raw=0.0800 weighted=-0.0931
13:   CONSIDERATION name=progress raw=0.2000 weighted=0.1700
13:   CONSIDERATION name=novelty raw=0.0000 weighted=-0.0000
13:   CONSIDERATION name=social raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=learned_success raw=0.5000 weighted=0.0000
13: SELECTION action=1 confidence=1.0000 expressed_emotion=sadness
13: 
13: === PROFILE curious agent=2 ===
13: STIMULUS predicate=1 positive=1 event_time=5 process_time=10
13: PERSONALITY persistence=0.3000 curiosity=0.9000 stability=0.6000 boldness=0.6500 sociability=0.5500
13: OBSERVE changed=1 urgency=2 decision_recommended=1
13: TRACE stage=observe detail=proposition processed weight=0.8985
13: TRACE stage=appraisal.consequence detail= weight=-0.6500
13: TRACE stage=appraisal.control detail= weight=0.5175
13: BELIEF predicate=1 positive=1 certainty=0.8379 source=0
13: BELIEF predicate=2 positive=1 certainty=0.7103 source=2
13: EMOTION name=joy intensity=0.0550 baseline=0.0550 excess=0.0000
13: EMOTION name=sadness intensity=0.2990 baseline=0.0243 excess=0.2747
13: EMOTION name=fear intensity=0.3049 baseline=0.0323 excess=0.2727
13: EMOTION name=anger intensity=0.0300 baseline=0.0300 excess=0.0000
13: EMOTION name=anxiety intensity=0.2498 baseline=0.0330 excess=0.2168
13: EMOTION name=aversion intensity=0.0200 baseline=0.0200 excess=0.0000
13: MOOD valence=-0.0113 activation=0.0128
13: CANDIDATE action=1 utility=0.2714
13:   CONSIDERATION name=cost raw=0.0000 weighted=-0.0000
13:   CONSIDERATION name=risk raw=0.4500 weighted=-0.3826
13:   CONSIDERATION name=progress raw=0.8000 weighted=0.4640
13:   CONSIDERATION name=novelty raw=0.4000 weighted=0.1900
13:   CONSIDERATION name=social raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=learned_success raw=0.5000 weighted=0.0000
13: CANDIDATE action=2 utility=-0.0320
13:   CONSIDERATION name=cost raw=0.0800 weighted=-0.0800
13:   CONSIDERATION name=risk raw=0.0800 weighted=-0.0680
13:   CONSIDERATION name=progress raw=0.2000 weighted=0.1160
13:   CONSIDERATION name=novelty raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=social raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=learned_success raw=0.5000 weighted=0.0000
13: SELECTION action=1 confidence=1.0000 expressed_emotion=sadness
13: 
13: === PROFILE stable-bold agent=3 ===
13: STIMULUS predicate=1 positive=1 event_time=5 process_time=10
13: PERSONALITY persistence=0.5500 curiosity=0.6500 stability=0.9500 boldness=0.9000 sociability=0.6000
13: OBSERVE changed=1 urgency=2 decision_recommended=1
13: TRACE stage=observe detail=proposition processed weight=0.8589
13: TRACE stage=appraisal.consequence detail= weight=-0.6500
13: TRACE stage=appraisal.control detail= weight=0.7875
13: BELIEF predicate=1 positive=1 certainty=0.7160 source=0
13: BELIEF predicate=2 positive=1 certainty=0.6128 source=2
13: EMOTION name=joy intensity=0.0700 baseline=0.0700 excess=0.0000
13: EMOTION name=sadness intensity=0.1290 baseline=0.0119 excess=0.1171
13: EMOTION name=fear intensity=0.1316 baseline=0.0148 excess=0.1169
13: EMOTION name=anger intensity=0.0213 baseline=0.0213 excess=0.0000
13: EMOTION name=anxiety intensity=0.2254 baseline=0.0140 excess=0.2114
13: EMOTION name=aversion intensity=0.0250 baseline=0.0250 excess=0.0000
13: MOOD valence=-0.0062 activation=0.0080
13: CANDIDATE action=1 utility=0.4370
13:   CONSIDERATION name=cost raw=0.0000 weighted=-0.0000
13:   CONSIDERATION name=risk raw=0.4500 weighted=-0.2970
13:   CONSIDERATION name=progress raw=0.8000 weighted=0.5840
13:   CONSIDERATION name=novelty raw=0.4000 weighted=0.1500
13:   CONSIDERATION name=social raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=learned_success raw=0.5000 weighted=0.0000
13: CANDIDATE action=2 utility=0.0132
13:   CONSIDERATION name=cost raw=0.0800 weighted=-0.0800
13:   CONSIDERATION name=risk raw=0.0800 weighted=-0.0528
13:   CONSIDERATION name=progress raw=0.2000 weighted=0.1460
13:   CONSIDERATION name=novelty raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=social raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=learned_success raw=0.5000 weighted=0.0000
13: SELECTION action=1 confidence=0.9849 expressed_emotion=anxiety
13: 
13: === PROFILE social-sensitive agent=4 ===
13: STIMULUS predicate=1 positive=1 event_time=5 process_time=10
13: PERSONALITY persistence=0.6000 curiosity=0.5500 stability=0.5500 boldness=0.4000 sociability=0.9500
13: OBSERVE changed=1 urgency=3 decision_recommended=1
13: TRACE stage=observe detail=proposition processed weight=0.9150
13: TRACE stage=appraisal.consequence detail= weight=-0.6500
13: TRACE stage=appraisal.control detail= weight=0.5000
13: BELIEF predicate=1 positive=1 certainty=0.6831 source=0
13: BELIEF predicate=2 positive=1 certainty=0.5865 source=2
13: EMOTION name=joy intensity=0.0610 baseline=0.0610 excess=0.0000
13: EMOTION name=sadness intensity=0.3155 baseline=0.0261 excess=0.2894
13: EMOTION name=fear intensity=0.3271 baseline=0.0422 excess=0.2848
13: EMOTION name=anger intensity=0.0312 baseline=0.0312 excess=0.0000
13: EMOTION name=anxiety intensity=0.2592 baseline=0.0400 excess=0.2192
13: EMOTION name=aversion intensity=0.0270 baseline=0.0270 excess=0.0000
13: MOOD valence=-0.0120 activation=0.0136
13: CANDIDATE action=1 utility=0.2118
13:   CONSIDERATION name=cost raw=0.0000 weighted=-0.0000
13:   CONSIDERATION name=risk raw=0.4500 weighted=-0.4582
13:   CONSIDERATION name=progress raw=0.8000 weighted=0.6080
13:   CONSIDERATION name=novelty raw=0.4000 weighted=0.0620
13:   CONSIDERATION name=social raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=learned_success raw=0.5000 weighted=0.0000
13: CANDIDATE action=2 utility=-0.0095
13:   CONSIDERATION name=cost raw=0.0800 weighted=-0.0800
13:   CONSIDERATION name=risk raw=0.0800 weighted=-0.0815
13:   CONSIDERATION name=progress raw=0.2000 weighted=0.1520
13:   CONSIDERATION name=novelty raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=social raw=0.0000 weighted=0.0000
13:   CONSIDERATION name=learned_success raw=0.5000 weighted=0.0000
13: SELECTION action=1 confidence=1.0000 expressed_emotion=sadness
13: 
13: AUDIT trace scenario PASS
```

La fuente ejecutable del escenario es [tests/audit_trace_scenario_tests.cpp](https://github.com/glitchaka/MSPA-Lib/blob/9c0a8f740fdac51dba886a19e2c0456d1349d95e/tests/audit_trace_scenario_tests.cpp).
