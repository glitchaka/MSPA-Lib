# MSPA-Lib — Arquitectura de código v0.1

> Estado: plan de arquitectura genérico. Este documento no contiene nombres, clases, escenarios ni reglas pertenecientes a proyectos consumidores.

## 1. Objetivo

Definir la organización del código de MSPA-Lib como biblioteca reutilizable, determinista, portable y desacoplada de cualquier host.

**Regla de frontera:** MSPA posee estado psicológico y lógica de interpretación/decisión. El host posee el mundo, las entidades reales, la percepción física, las acciones disponibles y su ejecución.

## 2. Lenguaje y ABI

Candidato principal de producción: **C++ nativo**.

Requisitos:
- ownership explícito;
- ausencia de dependencias obligatorias de motor;
- orden de iteración estable cuando afecte resultados;
- RNG determinista propio;
- validación defensiva de toda entrada externa;
- límites de memoria y complejidad;
- posibilidad de biblioteca estática o dinámica.

Una fachada C puede existir como ABI estable para bindings externos. No introduce lógica de dominio.

## 3. Modelo estructural

MSPA no modela jerarquías del mundo del host. Sólo conoce referencias contractuales:

- `AgentId`
- `EntityRef`
- `ConceptId`
- `PredicateId`
- `ActionTypeId`
- `ActionInstanceId`
- `ActionContinuityKey`
- `NamespaceId`
- `WorldId`

Las clases reales del host nunca forman parte del core.

## 4. Árbol de módulos previsto

```text
MSPA-Lib/
├── include/mspa/
│   ├── api/
│   ├── ids/
│   ├── model/
│   ├── runtime/
│   ├── config/
│   ├── diagnostics/
│   └── version/
├── src/
│   ├── base/
│   ├── semantics/
│   ├── psychology/
│   ├── decision/
│   ├── runtime/
│   ├── validation/
│   ├── persistence/
│   └── diagnostics/
├── connectors/
│   ├── c/
│   └── cpp/
├── schemas/
│   ├── core-vocabulary/
│   ├── domain-pack/
│   └── conformance/
├── tests/
│   ├── unit/
│   ├── conformance/
│   ├── determinism/
│   ├── security/
│   └── performance/
├── scenarios/conformance/
├── tools/sandbox/
└── docs/
```

Los escenarios de conformidad son abstractos y no representan personajes, juegos ni mundos concretos.

## 5. `base`

Responsabilidad: tipos fundamentales sin lógica psicológica.

Incluye:
- IDs fuertes;
- `PsychTime`;
- errores/resultados;
- límites y cuotas;
- RNG determinista;
- orden canónico;
- tipos normalizados;
- versiones de schema/runtime.

### PsychTime

```text
uint64 logical ticks
```

La escala la define el host. MSPA no acumula tiempo contractual mediante coma flotante.

### RNG

- sin RNG global compartido;
- stream independiente por agente;
- seed derivada de seed de runtime + identidad estable;
- procesar otro agente no altera la secuencia del primero.

## 6. `semantics`

Responsabilidad: lenguaje semántico portable.

### VocabularyRegistry
Registra vocabulario base, namespaces externos, predicados, conceptos, tipos de acción y schemas. Es inmutable durante un batch.

### PropositionModel
Representa afirmaciones mediante predicado, argumentos por rol, polaridad, tiempo, calificadores y procedencia.

### PredicateSchemaRegistry
Declara roles, agencia, consecuencias semánticas, relevancia, compatibilidad/contradicción, namespace y versión. Un schema nunca fija directamente una emoción.

### InferenceEngine
Motor declarativo acotado:
- no Turing-completo;
- sin ejecución arbitraria de código;
- sin recursión ilimitada;
- presupuesto máximo por estímulo;
- profundidad máxima;
- máximo de hipótesis;
- terminación garantizada bajo cuotas.

Las reglas específicas de un dominio viven fuera del core y son consumidas como datos.

## 7. `psychology`

No depende de clases del host.

### Personality
Seis ejes base: sociabilidad, estabilidad, empatía, persistencia, curiosidad y audacia. Modifican sensibilidades y curvas, no seleccionan acciones directamente.

### Values
`ConceptId` + intensidad. Los catálogos concretos son externos.

### Beliefs
Cada creencia mantiene proposición, certeza, fuente, origen, carga emocional, timestamps y evidencia resumida. Permite crear, reforzar, debilitar, contradecir, fusionar, compactar y recuperar.

### Appraisal
Evalúa relevancia, consecuencia, agencia, intención percibida, certeza, control, expectativa, norma y relaciones.

### EmotionStore
Instancias de alegría, tristeza, miedo, ira, ansiedad y aversión, con causa, objetivo, intensidad base y timestamps.

### Mood
Valencia + activación con actualización diferida.

### Goals
Metas activas, suspendidas, bloqueadas, cumplidas o abandonadas.

### Memory
Memorias ordinarias, consolidadas y vitales que preservan interpretación subjetiva.

### Relationships
Relaciones direccionales: afecto, confianza, respeto y familiaridad.

### Learning
Primera versión limitada:

```text
ActionType + ContextTags -> OutcomeTags
```

Mantiene estadísticas y expectativa acotada.

## 8. `decision`

Elige únicamente entre opciones ofrecidas por el host.

### ActionOption
Contiene tipo semántico, instancia, `ActionContinuityKey`, actor, objetivo, costes, riesgos, efectos esperados, tags/contexto y progreso opcional.

### Considerations
Cada acción declara sólo las consideraciones aplicables: necesidad, meta, valor, emoción, relación, memoria, norma, riesgo, coste y expectativa aprendida.

### ResponseCurves
Candidato inicial: curvas lineales por tramos, serializables, editables y deterministas.

### UtilityComposer
Combina contribuciones, condicionantes, histéresis, continuidad y ruido pequeño determinista. La fórmula exacta se valida por conformidad antes de congelarse.

### Hysteresis
Usa continuidad, progreso, coste de interrupción y prioridad de nueva información urgente.

## 9. `runtime`

```text
Runtime
  ConfigSnapshot
  PackRegistrySnapshot
  AgentStore
  Diagnostics
```

### Operaciones fundamentales
- `OBSERVE`
- `DECIDE`
- `REPORT`
- `ADVANCE_TO`
- `EXPORT`
- `IMPORT`

`REACT` puede existir como wrapper de `OBSERVE + DECIDE`.

### OBSERVE
Entrada: estímulo + timestamp.
Salida mínima: `state_changed`, `decision_recommended`, `urgency`, `current_action_threatened`.

Urgencia: `None`, `Normal`, `Soon`, `Immediate`.

### DECIDE
Entrada: agente, timestamp, opciones del host y continuidad actual opcional.
Salida: acción seleccionada, objetivo, confianza, intención, expresión observable, afirmaciones a transmitir y explicación causal.

### REPORT
Recibe el resultado real de una acción y actualiza expectativas, aprendizaje, emociones, creencias y metas.

### ADVANCE_TO
Actualización temporal diferida; no es un tick obligatorio.

## 10. Batch

El resultado de un batch debe ser semánticamente equivalente al procesamiento individual en orden canónico.

Durante un batch son mutables sólo el estado privado de cada agente y buffers locales. Packs, schemas, vocabulario, curvas, reglas, configuración y cachés compartidas visibles al algoritmo son de sólo lectura.

## 11. `validation`

Toda entrada externa se considera no confiable.

Valida:
- estados importados;
- Domain Packs;
- proposiciones;
- reglas;
- namespaces/versiones;
- límites de tamaño y complejidad;
- referencias estructurales.

Jerarquía de cuotas:
1. hard limits de MSPA;
2. runtime policy del host;
3. recomendaciones del pack.

Un pack nunca eleva límites del host o del core.

## 12. `persistence`

MSPA ofrece un modelo portable `AgentPsychState` con versionado, namespaces, referencias externas preservables, validación y migraciones controladas. El host decide el formato físico de almacenamiento o transporte.

## 13. `diagnostics`

Debe exponer estímulos procesados, cambios de creencias, inferencias, memorias activadas, emociones relevantes, opciones consideradas, contribuciones de utilidad y razón de la decisión. Los diagnósticos no alteran la simulación.

## 14. Conectores

Los conectores incluidos en MSPA son genéricos:
- API C++ pública;
- fachada C opcional.

Adaptadores para motores, juegos o frameworks concretos viven fuera del core.

## 15. Domain Packs

MSPA define contrato, schema y validador. Los Domain Packs concretos pertenecen a repositorios consumidores.

## 16. Conformidad

La suite usa escenarios abstractos para validar:
- divergencia de creencias ante la misma evidencia;
- evidencia contradictoria;
- reapreciación sin cambio factual;
- influencia externa interpretable;
- referencias externas no resolubles;
- aprendizaje causal simple;
- continuidad e histéresis;
- equivalencia batch/individual;
- determinismo;
- rechazo seguro de entradas hostiles.

## 17. Orden de implementación

1. **Base contractual:** IDs, tiempo, RNG, errores, cuotas y tipos públicos.
2. **Runtime mínimo:** AgentStore, OBSERVE, DECIDE mínimo, REPORT y diagnostics.
3. **Conocimiento y afecto:** creencias, appraisal, emociones y actualización por evidencia.
4. **Decisión:** consideraciones, curvas, utilidad e histéresis.
5. **Continuidad psicológica:** memoria, relaciones, metas, ánimo y reapreciación.
6. **Inferencia y aprendizaje:** reglas declarativas, aprendizaje causal simple y expectativas.
7. **Portabilidad y seguridad:** export/import, validación, cuotas y referencias externas.
8. **Batch y rendimiento:** API batch, paralelización opcional y equivalencia.
9. **Conectores:** API C++ y fachada C opcional.

## 18. Dependencias permitidas

```text
base
 ↑
semantics
 ↑
psychology
 ↑
decision
 ↑
runtime
```

En paralelo:

```text
validation  -> base + semantics + model
persistence -> base + model
diagnostics -> model + decision trace
connectors  -> public api only
```

## 19. Invariante de diseño

Cualquier necesidad de añadir nombres, clases, reglas o escenarios de un proyecto concreto dentro del core indica una violación de frontera. Esa lógica debe permanecer en el host o en datos externos compatibles con el contrato MSPA.