# MSPA 1.1.2 — Auditoría de seguimiento externa

## Origen y alcance

Este seguimiento reproduce el archivo `audit_followup_tests.cpp` entregado por el auditor externo después de la fase 2. El objetivo era cerrar tres debilidades de cobertura sin ajustar criterios después de ver resultados:

- **H\***: la histéresis debe amortiguar ruido sin bloquear al agente sobre una opción claramente peor;
- **C\***: la estabilidad debe afectar la velocidad de recuperación emocional, no sólo el baseline tónico final;
- **P\***: la personalidad debe cambiar decisiones reales, no sólo creencias o emociones.

El test usa sólo la API pública C++ de MSPA.

## Historial de ejecución

### 1. Primera compilación

El archivo externo contenía una incompatibilidad nominal: dentro de `test_decay_speed`, `r` es `std::unique_ptr<Runtime>` pero una línea invocaba `r.advance_to(...)`. Se corrigió exclusivamente a `r->advance_to(...)`. No se modificó ningún criterio, umbral ni escenario.

Commit con la corrección nominal: `f5e8ba3bd7a50e02331874289c518b6c0f7ff2f0`.

### 2. Primera ejecución conductual

Con el test compilando, la suite falló en H*:

```text
FAIL: a clearly better option (gap 0.50) must be adopted within 3 decisions
```

Los otros tests existentes seguían pasando. El hallazgo demostró que la cobertura anterior de histéresis verificaba reducción de oscilación, pero no verificaba que la continuidad pudiera ser abandonada ante una alternativa claramente superior.

### 3. Causa

`Runtime::decide` aplica al candidato incumbente tanto `HysteresisPolicy::continuity_bonus` como `HysteresisPolicy::switch_threshold` como contribuciones positivas a utilidad. Las fórmulas estaban calibradas individualmente como barreras completas, por lo que su suma podía superar incluso una mejora de utilidad de `0.50` para un agente con alta persistencia/estabilidad.

### 4. Corrección

Se mantuvieron las dos contribuciones para conservar la explicabilidad aritmética de la traza, pero cada una aporta ahora la mitad de su presupuesto de inercia anterior. La suma conserva amortiguación fuerte frente a ruido near-tie y evita el doble conteo que producía bloqueo.

Commit corregido y auditado: `25978c1ca2d19ad7b6493ee35d04cbc3b09ae155`.

## Resultado final

Workflow de auditoría: https://github.com/glitchaka/MSPA-Lib/actions/runs/40211349844
Workflow CI multiplataforma: https://github.com/glitchaka/MSPA-Lib/actions/runs/40211349958

Sobre `25978c1ca2d19ad7b6493ee35d04cbc3b09ae155`:

```text
mspa_audit_followup_tests: AUDIT followup PASS
18/18 CTest PASS
```

### H* — histéresis sin bloqueo

**PASS**.

Se verifican simultáneamente:

- opción claramente mejor con gap `0.50` adoptada en no más de 3 decisiones;
- mejoras mayores nunca tardan más que mejoras menores;
- una vez adoptada la mejor opción no hay flip-flop de regreso;
- con ruido determinista near-tie, continuidad produce <=10 cambios;
- sin continuidad se producen >=30 cambios;
- la histéresis reduce los cambios al menos 3 veces.

El mutation test existente que anula la histéresis continúa siendo eliminado por la suite.

### C* — velocidad de decay emocional

**PASS**.

Se verifica:

- dos agentes con personalidad idéntica producen curvas idénticas;
- miedo no aumenta sin nuevos estímulos;
- comparando exceso episódico normalizado sobre el baseline propio, mayor estabilidad conserva una fracción menor del miedo al mismo tiempo transcurrido;
- `ADVANCE_TO` paso a paso y salto único concuerdan;
- comenzar en `PsychTime(0)` o en un tiempo desplazado produce el mismo decay para el mismo intervalo.

Esto demuestra velocidad de recuperación, no sólo diferencias en el baseline tónico.

### P* — personalidad cambia decisiones

**PASS**.

Con dos agentes que sólo difieren en `boldness`:

- el agente audaz asigna mayor utilidad relativa a la opción arriesgada en todo el barrido;
- nunca aparece la inversión patológica donde el tímido acepta un riesgo que el audaz rechaza;
- la decisión es monótona al aumentar el coste de la opción segura;
- existe al menos un punto del barrido donde la personalidad cambia la acción seleccionada realmente.

## Validación adicional

En el SHA auditado:

- GCC Release: PASS;
- Clang + ASan/UBSan: PASS;
- MSVC Release: PASS;
- shared build: PASS;
- fuzz smoke: PASS;
- mutation testing: PASS, incluyendo histéresis;
- benchmark y CTest verbose quedaron publicados como artifact del workflow de auditoría.

## Conclusión

El seguimiento encontró un defecto real que la fase 2 no detectó: la histéresis podía convertirse en bloqueo de acción. El criterio externo no se relajó. Tras corregir la política, H*, C* y P* pasan juntos contra la API pública en GCC, Clang+sanitizers y MSVC.
