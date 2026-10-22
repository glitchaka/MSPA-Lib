# MSPA Interaction Contract v0.2

## Estado

Contrato lógico candidato. No define sintaxis concreta, ABI ni formato de serialización.

Este documento fija qué intercambian el host y MSPA y qué garantías lógicas deben conservarse entre implementaciones. No contiene escenarios, nombres ni reglas de proyectos consumidores.

## 1. Operaciones fundamentales

1. `OBSERVE`
2. `DECIDE`
3. `REPORT`
4. `ADVANCE_TO`
5. `EXPORT / IMPORT`

Todas las operaciones relevantes llevan tiempo lógico explícito.

## 2. OBSERVE

`OBSERVE(agent, stimulus, time)`

El host entrega sólo información a la que el agente tuvo acceso.

Un `Stimulus` puede representar:
- evento físico;
- testimonio;
- resultado observado;
- estado interno;
- cambio ambiental;
- influencia externa;
- comunicación;
- recordatorio contextual.

Campos conceptuales:
- `StimulusId`
- `StimulusType`
- `source`
- `target`
- `propositions[]`
- `perceptual_confidence`
- `salience`
- `namespace`
- `timestamp`
- metadatos opcionales validados.

Salida:
- `state_changed`
- `decision_recommended`
- `urgency`
- `current_action_threatened`
- `diagnostic_token` opcional.

`OBSERVE` no selecciona acciones.

## 3. DECIDE

`DECIDE(agent, available_actions, current_action?, time)`

Puede llamarse sin un `OBSERVE` inmediatamente anterior.

### ActionOption

Debe contener:
- `ActionTypeId`
- `ActionInstanceId`
- `ActionContinuityKey`
- actor
- objetivo(s)
- semántica registrada
- efectos esperados
- costes
- riesgos
- tags
- consideraciones registradas
- restricciones físicas ya resueltas por el host
- progreso opcional.

`ActionContinuityKey` debe permanecer estable mientras la opción represente la misma actividad lógica.

### Decision

Puede contener:
- `selected_action`
- `selected_target`
- `confidence`
- `intention`
- `expressed_emotion`
- `outgoing_statements[]`
- `continuity_expectation`
- `explanation`.

MSPA sólo selecciona entre acciones ofrecidas por el host.

## 4. REPORT

`REPORT(agent, action_result, time)`

`ActionResult` puede incluir:
- acción intentada;
- continuidad asociada;
- éxito, fracaso o resultado parcial;
- resultado observable;
- `OutcomeTags`;
- consecuencias inesperadas;
- terceros afectados;
- motivo de interrupción;
- costes reales;
- proposiciones resultantes.

MSPA puede usar el reporte para actualizar metas, emociones, expectativas, aprendizaje, relaciones, memoria y creencias.

## 5. ADVANCE_TO

`ADVANCE_TO(agent, target_time)`

No es un tick obligatorio. Sirve para materializar efectos temporales diferidos, caducar metas y detectar si el paso del tiempo recomienda una nueva decisión.

## 6. EXPORT / IMPORT

`EXPORT` produce un `AgentPsychState` portable y versionado.

`IMPORT` valida:
1. versión;
2. estructura;
3. namespaces y packs;
4. límites de tamaño y complejidad;
5. referencias;
6. datos opacos preservables;
7. ausencia de contenido no autorizado.

La autenticación pertenece al host. La seguridad estructural del estado pertenece a MSPA.

## 7. AgentPsychState

Puede contener:
- identidad psicológica;
- personalidad;
- valores;
- metas;
- creencias;
- emociones activas;
- ánimo;
- relaciones;
- memorias;
- expectativas aprendidas;
- referencias externas;
- estado RNG suficiente para reproducibilidad;
- versión;
- namespaces requeridos.

## 8. Proposition

Una proposición contiene:
- `PredicateId`
- roles y argumentos
- polaridad
- tiempo/intervalo
- calificadores
- namespace
- procedencia.

MSPA debe poder detectar igualdad semántica básica, contradicción, compatibilidad y relaciones de evidencia declaradas.

## 9. Predicate Schema

Todo predicado interpretable debe registrar:
- roles válidos;
- rol agente;
- posibles objetivos;
- categorías de efecto;
- intencionalidad;
- relevancias posibles;
- tags de aprendizaje;
- pistas de appraisal;
- mapeos opcionales al vocabulario base.

Un predicado desconocido puede conservarse como dato opaco.

## 10. Reglas de inferencia

Las reglas son datos declarativos, serializables y versionables. No ejecutan código arbitrario.

El runtime impone:
- profundidad máxima;
- máximo de reglas por estímulo;
- máximo de hipótesis;
- presupuesto lógico;
- terminación garantizada.

Las reglas producen hipótesis con certeza base combinada después con evidencia y sesgos del agente.

## 11. Consideraciones de utilidad

Cada tipo de acción declara sólo consideraciones relevantes. Cada consideración transforma una entrada normalizada mediante una curva serializable.

Se distinguen:
- contribuciones de utilidad;
- penalizaciones/condicionantes;
- restricciones físicas ya eliminadas por el host.

La fórmula final se valida mediante conformidad antes de congelarse.

## 12. Histéresis

La continuidad se reconoce mediante `ActionContinuityKey` y puede considerar progreso, coste de interrupción, persistencia de meta, compromiso y urgencia nueva.

Si el host no proporciona continuidad estable, MSPA no garantiza histéresis robusta.

## 13. Batch

Toda operación individual puede tener forma batch equivalente.

Garantía:

> Procesar un batch debe ser semánticamente equivalente a procesar sus agentes individualmente en orden canónico.

El orden canónico usa referencias estables y no depende del scheduling de threads ni del orden de hash tables.

## 14. Determinismo

Cada agente dispone de un stream pseudoaleatorio independiente o derivable de forma estable.

No se permite depender de:
- orden de iteración no especificado;
- scheduling de threads;
- RNG global compartido;
- comportamiento matemático no definido por la política de conformidad.

## 15. Tiempo

Toda llamada relevante lleva timestamp lógico entero. Los decaimientos deben poder evaluarse perezosamente.

## 16. Aprendizaje causal inicial

Primera versión:

```text
ActionType + ContextTags -> OutcomeTags
```

Cada patrón conserva estadísticas limitadas. El Domain Pack controla qué tags son válidos para evitar explosión combinatoria.

## 17. Entrada no confiable

Estados y packs externos se tratan como datos hostiles. Se imponen cuotas a tamaño total, creencias, memorias, metas, relaciones, reglas, profundidad de proposiciones, strings, patrones de aprendizaje y derivaciones máximas.

## 18. Compatibilidad de packs

Cada pack declara `pack_id`, versión, namespace, dependencias, compatibilidad, vocabulario, schemas, reglas y curvas.

Conceptos desconocidos pueden permanecer opacos y referencias externas pueden mantenerse sin resolución local.

## 19. Degradación

Debe existir comportamiento definido cuando módulos opcionales estén desactivados:
- sin inferencia: sólo percepción/testimonio directo;
- sin memoria: no hay reactivación episódica;
- sin aprendizaje: no se actualizan expectativas causales;
- sin expresión: no se produce salida expresiva;
- sin relaciones: appraisal social usa valores neutrales configurables;
- sin ánimo: emociones operan directamente.

## 20. Conformidad

El contrato no se congela hasta validar, con escenarios abstractos y neutrales:
- divergencia de creencias;
- evidencia contradictoria;
- reapreciación;
- influencia externa;
- referencias no resolubles;
- continuidad/histéresis;
- batch vs individual;
- determinismo;
- estado hostil;
- aprendizaje causal simple.

La suite de conformidad no contiene personajes, juegos, lugares ni reglas de proyectos consumidores.