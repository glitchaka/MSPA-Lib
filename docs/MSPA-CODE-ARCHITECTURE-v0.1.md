# MSPA-Lib — Arquitectura de código v0.1

> Estado: **plan de arquitectura**. Este documento no autoriza todavía implementación de producción.

## 1. Objetivo

Definir cómo debe organizarse el código de MSPA-Lib para convertir la especificación lógica v2.4 y el Interaction Contract v0.2 en una biblioteca reutilizable, determinista, portable y desacoplada de los proyectos consumidores.

MSPA no contiene lógica específica de `dungeon-sims`, Isla Viva ni ningún otro juego. Los proyectos consumidores reciben los conectores de MSPA y realizan externamente el mapeo de sus entidades, eventos y acciones al contrato MSPA.

Principio de frontera:

> **MSPA posee el estado psicológico y la lógica de interpretación/decisión. El host posee el mundo, las entidades reales y la ejecución de las acciones.**

---

# 2. Lenguaje y frontera binaria prevista

## 2.1 Núcleo de producción

Candidato principal: **C++ nativo**.

Motivos:

- integración directa con hosts nativos;
- control explícito de memoria y asignaciones;
- facilidad para construir biblioteca estática o dinámica;
- buena compatibilidad con motores y toolchains de juegos;
- permite una fachada C estable para otros lenguajes.

## 2.2 Fachada C opcional

La API C no sustituye a la API C++ interna. Actúa como frontera binaria estable para bindings externos.

Consumidores potenciales:

- C++ directo;
- Godot/GDExtension mediante wrapper;
- C# mediante P/Invoke u otro binding;
- Python para herramientas y pruebas;
- otros hosts capaces de consumir ABI C.

No se diseñará una API específica para ningún motor dentro del core.

---

# 3. Principio estructural: composición, no jerarquía de entidades

MSPA no tendrá una jerarquía del tipo:

```text
Entity
 ├─ Human
 ├─ Animal
 ├─ Building
 └─ Ship
```

Eso pertenece al host.

MSPA sólo distingue conceptos contractuales:

- `AgentId`: entidad con estado psicológico MSPA;
- `EntityRef`: cualquier entidad que puede aparecer en una proposición, recuerdo, relación o acción;
- `ConceptId`: concepto semántico;
- `PredicateId`: relación/predicado semántico;
- `ActionTypeId`: tipo semántico de acción;
- `ActionInstanceId`: ejecución concreta;
- `NamespaceId`: origen semántico/espacio de nombres;
- `WorldId`: espacio de identidad externo.

Un `Resident`, `Animal`, `Prop`, `Ship` o cualquier clase propia de un juego sólo entra a MSPA a través de esos identificadores y metadatos.

---

# 4. Árbol de módulos previsto

```text
MSPA-Lib/
│
├── include/
│   └── mspa/
│       ├── api/
│       ├── ids/
│       ├── model/
│       ├── runtime/
│       ├── config/
│       ├── diagnostics/
│       └── version/
│
├── src/
│   ├── base/
│   ├── semantics/
│   ├── psychology/
│   ├── decision/
│   ├── runtime/
│   ├── validation/
│   ├── persistence/
│   └── diagnostics/
│
├── connectors/
│   ├── c/
│   └── cpp/
│
├── schemas/
│   ├── core-vocabulary/
│   ├── domain-pack/
│   └── conformance/
│
├── tests/
│   ├── unit/
│   ├── conformance/
│   ├── determinism/
│   ├── security/
│   └── performance/
│
├── scenarios/
│   ├── mara/
│   ├── inspiration/
│   └── traveler/
│
├── tools/
│   └── sandbox/
│
└── docs/
```

La estructura física podrá ajustarse durante el sandbox, pero las fronteras lógicas no deben mezclarse.

---

# 5. Módulo `base`

Responsabilidad: tipos fundamentales sin lógica psicológica.

Contiene conceptualmente:

- IDs fuertes;
- `PsychTime`;
- resultados/errores;
- límites y cuotas;
- RNG determinista;
- utilidades de orden canónico;
- tipos numéricos normalizados;
- versiones de schema/runtime.

## 5.1 PsychTime

Tipo contractual previsto:

```text
uint64 logical ticks
```

La escala la define el host, pero MSPA no acumula tiempo usando `float`.

## 5.2 IDs fuertes

No se deben intercambiar enteros sin tipo entre subsistemas.

`AgentId`, `ActionInstanceId`, `PredicateId`, etc. deben ser tipos distintos a nivel de C++ para evitar mezclas accidentales.

## 5.3 RNG

- sin generador global compartido;
- stream independiente por agente;
- seed derivada de seed de runtime + identidad estable;
- el procesamiento de otro agente no debe alterar la secuencia aleatoria del primero.

---

# 6. Módulo `semantics`

Responsabilidad: lenguaje semántico portable.

Subcomponentes:

## 6.1 Vocabulary Registry

Registra:

- vocabulario base MSPA;
- namespaces de hosts;
- predicados;
- conceptos;
- tipos de acción;
- schemas.

Durante un batch activo es **inmutable**.

## 6.2 Proposition Model

Representa afirmaciones mediante:

- predicado;
- argumentos por rol;
- polaridad;
- tiempo;
- calificadores;
- procedencia.

No contiene conocimiento específico de un juego.

## 6.3 Predicate Schema Registry

Cada predicado de dominio declara:

- roles válidos;
- roles de agencia;
- clases de consecuencias semánticas;
- pistas de relevancia;
- compatibilidad/contradicción básica;
- namespace y versión.

Un schema no produce directamente una emoción.

## 6.4 Inference Engine

Motor declarativo acotado.

Requisitos:

- no Turing-completo;
- sin ejecución arbitraria de código;
- sin recursión ilimitada;
- presupuesto máximo por estímulo;
- profundidad máxima de derivación;
- número máximo de hipótesis producidas;
- terminación garantizada bajo las cuotas configuradas.

Las reglas pertenecen al Domain Pack del host, pero el evaluador vive en MSPA.

---

# 7. Módulo `psychology`

Contiene el estado y transformaciones psicológicas de cada agente.

No debe depender de ninguna clase del host.

Submódulos:

## 7.1 Personality

Seis ejes base configurables mediante datos:

- sociabilidad;
- estabilidad;
- empatía;
- persistencia;
- curiosidad;
- audacia.

La personalidad modifica sensibilidades y curvas, no selecciona acciones directamente.

## 7.2 Values

Valores identificados por `ConceptId` + intensidad.

Los valores concretos pertenecen al pack del host.

## 7.3 Beliefs

Cada creencia mantiene:

- proposición;
- certeza;
- fuente;
- origen;
- carga emocional;
- timestamps;
- evidencias relacionadas.

Funciones internas:

- crear;
- reforzar;
- debilitar;
- contradecir;
- fusionar cuando proceda;
- olvidar/compactar;
- recuperar por relevancia.

## 7.4 Appraisal

Evalúa una proposición subjetiva frente al estado del agente.

Usa:

- relevancia;
- consecuencia;
- agencia;
- intención percibida;
- certeza;
- control;
- expectativa/novedad si existe;
- norma;
- relaciones.

No conoce eventos concretos del host.

## 7.5 Emotion Store

Instancias de:

- alegría;
- tristeza;
- miedo;
- ira;
- ansiedad;
- aversión.

Cada instancia posee causa, objetivo, intensidad base y timestamps para decaimiento perezoso.

## 7.6 Mood

Estado lento basado en:

- valencia;
- activación.

No se recalcula continuamente: usa timestamps y actualización diferida.

## 7.7 Goals

Metas activas, suspendidas, bloqueadas, cumplidas o abandonadas.

Los compromisos son un origen de meta, no un sistema separado.

## 7.8 Memory

Memorias ordinarias, consolidadas y vitales.

Conservan interpretación subjetiva, no copia completa del mundo.

## 7.9 Relationships

Direccionales:

- afecto;
- confianza;
- respeto;
- familiaridad.

`A → B` no implica `B → A`.

## 7.10 Learning

Primera versión deliberadamente limitada:

```text
ActionType + ContextTags -> OutcomeTags
```

Mantiene contadores y certeza/expectativa.

No intenta aprendizaje causal general ni modelos estadísticos complejos en la primera implementación.

---

# 8. Módulo `decision`

Responsabilidad: elegir entre opciones ofrecidas por el host.

MSPA nunca inventa una acción ejecutable que el host no haya ofrecido.

## 8.1 ActionOption

Debe poder identificar:

- tipo semántico;
- instancia concreta;
- `ActionContinuityKey` estable;
- actor;
- objetivo;
- costes;
- riesgos;
- efectos esperados;
- tags/contexto;
- estado de acción en curso cuando corresponda.

## 8.2 Considerations

Cada tipo de acción declara qué consideraciones le son aplicables.

Ejemplos conceptuales:

- necesidad;
- meta;
- valor;
- emoción;
- relación;
- memoria;
- norma;
- riesgo;
- coste;
- expectativa aprendida.

No se calculan todos los factores para todas las acciones.

## 8.3 Response Curves

Candidato inicial:

- curvas lineales por tramos;
- serializables;
- editables;
- deterministas;
- sin depender de funciones trascendentales para el comportamiento configurable.

## 8.4 Utility Composer

Combina:

- contribuciones positivas/negativas;
- condicionantes que reducen viabilidad;
- histéresis;
- continuidad de acción;
- ruido pequeño determinista.

La fórmula exacta todavía debe validarse en sandbox.

## 8.5 Hysteresis

La continuidad usa `ActionContinuityKey`, progreso, coste de interrupción y prioridad de nuevas amenazas.

Una acción nueva debe superar suficientemente a la actual para interrumpirla, salvo urgencia crítica.

---

# 9. Módulo `runtime`

Es el orquestador público de MSPA.

Conceptualmente mantiene:

```text
Runtime
  ConfigSnapshot
  PackRegistrySnapshot
  AgentStore
  Diagnostics
```

## 9.1 AgentStore

MSPA posee el estado psicológico activo de los agentes registrados.

El host conserva la identidad y datos reales de la entidad correspondiente.

## 9.2 Operaciones fundamentales

- `OBSERVE`
- `DECIDE`
- `REPORT`
- `ADVANCE_TO`
- `EXPORT`
- `IMPORT`

`REACT` puede existir como wrapper de `OBSERVE + DECIDE`, nunca como operación fundamental.

## 9.3 OBSERVE

Entrada: estímulo + timestamp.

Puede actualizar:

- creencias;
- emociones;
- relaciones;
- metas;
- memoria;
- aprendizaje.

Salida mínima:

- si cambió el estado;
- recomendación de reevaluación;
- urgencia;
- si la acción actual quedó amenazada.

Urgencia contractual:

- `None`;
- `Normal`;
- `Soon`;
- `Immediate`.

## 9.4 DECIDE

Entrada:

- AgentId;
- timestamp;
- opciones reales del host;
- acción en curso/continuidad cuando proceda.

Salida:

- acción seleccionada;
- target;
- confianza;
- intención psicológica;
- expresión observable;
- afirmaciones que desea transmitir;
- explicación causal.

## 9.5 REPORT

El host comunica el resultado real de una acción.

MSPA actualiza expectativas, aprendizaje, emociones, creencias y metas.

## 9.6 ADVANCE_TO

No es un tick obligatorio.

Sirve para:

- actualizar agentes dormidos;
- caducar metas;
- aplicar paso temporal diferido;
- descubrir que un estado requiere nueva decisión.

Todas las demás llamadas también llevan timestamp.

---

# 10. Procesamiento batch

Debe existir arquitectura para llamadas por lote.

Regla contractual:

> El resultado de un batch debe ser equivalente al procesamiento individual en orden canónico definido por MSPA.

Durante un batch:

### Mutable

- estado privado de cada agente procesado;
- buffers locales de diagnóstico.

### Read-only

- Domain Packs;
- schemas;
- vocabulario;
- curvas;
- reglas de inferencia;
- configuración global;
- tablas compartidas de internado/cachés semánticos visibles al algoritmo.

Los cambios de pack/configuración sólo ocurren entre batches.

---

# 11. `validation`

Todo dato externo se considera no confiable.

Responsabilidades:

- validar AgentPsychState importado;
- validar Domain Packs;
- validar proposiciones;
- validar reglas declarativas;
- validar namespaces/versiones;
- aplicar cuotas;
- rechazar referencias estructuralmente inválidas;
- preservar referencias externas válidas aunque no sean resolubles localmente.

## 11.1 Jerarquía de cuotas

1. hard limits compilados en MSPA;
2. runtime policy del host;
3. recomendaciones del pack.

Un pack nunca puede elevar los límites impuestos por el host o por MSPA.

Si el host no configura cuotas, se utilizan defaults conservadores de MSPA.

---

# 12. `persistence`

El core no debe imponer inicialmente un formato de archivo del juego.

Sí debe ofrecer un modelo portable de estado:

```text
AgentPsychState
```

Debe contener únicamente datos necesarios para reconstruir el estado psicológico.

Requisitos:

- versionado explícito;
- namespaces;
- referencias externas preservables;
- validación antes de importar;
- migraciones controladas entre versiones compatibles.

El host decide cómo transportar o guardar la representación serializada final.

---

# 13. `diagnostics`

La explicabilidad es parte del runtime.

Debe poder responder conceptualmente:

- qué percibió;
- qué creencia creó o cambió;
- qué inferencia se ejecutó;
- qué memoria se activó;
- qué emociones influyeron;
- qué opciones se consideraron;
- qué consideración aportó o restó utilidad;
- por qué se mantuvo/interrumpió una acción;
- por qué se aceptó/rechazó evidencia.

Dos niveles:

## Compact explanation

Para UI/debug normal.

## Full trace

Para sandbox y diagnóstico profundo.

El tracing completo debe poder desactivarse para no penalizar producción.

---

# 14. `connectors`

Los conectores pertenecen a MSPA, pero nunca contienen lógica específica de un juego.

## 14.1 C++ connector

API idiomática para hosts C++.

## 14.2 C connector

Fachada ABI estable con handles opacos.

Bindings futuros pueden construirse encima sin alterar el core.

## 14.3 Qué NO vive aquí

No deben existir dentro de MSPA:

- `DungeonSimsPsychPack`;
- `IslaVivaPsychPack`;
- adaptadores para `Resident`, `Ship`, etc.;
- reglas de un juego particular.

Eso vive en el repositorio consumidor.

---

# 15. Domain Packs

MSPA define el **formato**, nunca los packs concretos de los juegos.

Un host puede proporcionar externamente un Domain Pack con:

- namespace;
- predicados;
- schemas de predicados;
- tipos de acción;
- reglas declarativas de inferencia;
- tags de contexto/aprendizaje;
- curvas de respuesta específicas;
- conceptos/valores;
- metadatos de compatibilidad.

El pack se carga y valida mediante MSPA, pero pertenece al proyecto consumidor.

---

# 16. Dependencias permitidas entre módulos

Dirección prevista:

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

Módulos laterales:

```text
validation -> base + semantics + model
persistence -> model + validation
 diagnostics -> todos mediante interfaces de lectura
connectors -> runtime público
```

Reglas:

- `base` no depende de nada psicológico;
- `semantics` no depende de `decision`;
- `psychology` no depende del host;
- `decision` no ejecuta acciones;
- `runtime` orquesta, no contiene algoritmos psicológicos duplicados;
- conectores nunca saltan el runtime para modificar internals.

---

# 17. Ownership y memoria

Principios previstos:

- `Runtime` posee estados de agentes activos;
- `AgentState` posee sus creencias, recuerdos, emociones, metas y relaciones;
- registros de packs/schemas son snapshots inmutables durante procesamiento;
- el host conserva entidades reales y acciones ejecutables;
- referencias externas a entidades son IDs, nunca punteros del host;
- ningún puntero/referencia al objeto real del juego persiste dentro de MSPA.

Objetivo: poder exportar un agente sin depender de direcciones de memoria del host.

---

# 18. Manejo de errores

Distinguir al menos:

- error de contrato del host;
- dato inválido;
- cuota excedida;
- referencia desconocida pero tolerable;
- incompatibilidad de pack;
- versión incompatible;
- inferencia agotó presupuesto;
- no hay acción seleccionable;
- estado corrupto/no importable.

Los errores recuperables no deben confundirse con asserts internos.

---

# 19. Determinismo

Requisitos de implementación:

- tiempo entero;
- RNG por agente;
- orden canónico;
- no depender del orden de `unordered_map` o equivalentes;
- configuración inmutable durante batches;
- curvas reproducibles;
- operaciones de compactación/olvido con desempate determinista;
- IDs estables;
- mismas entradas + mismo estado + misma versión + misma configuración => mismo resultado observable dentro de la plataforma/garantía numérica declarada.

La garantía exacta multiplataforma debe validarse antes de prometer bit-identical output.

---

# 20. Arquitectura del sandbox

El sandbox no debe convertirse en dependencia del core.

Objetivo:

- ejecutar escenarios neutrales;
- ajustar utility;
- medir divergencia entre perfiles;
- observar acumulación de creencias/memorias;
- medir costes;
- validar determinismo.

Los escenarios viven como datos y deben poder ejecutarse posteriormente contra la implementación C++ de producción.

Escenarios iniciales:

1. Mara — cuatro perfiles;
2. inspiración externa;
3. viajero entre namespaces/mundos;
4. contradicción de fuentes;
5. aprendizaje de una acción;
6. histéresis entre acciones casi empatadas.

---

# 21. Orden de implementación propuesto

No se comienza por emociones ni por UI.

## Etapa 0 — suite de escenarios

Definir datos neutrales de los escenarios de conformidad.

## Etapa 1 — base y tipos contractuales

- IDs;
- PsychTime;
- Proposition;
- Stimulus;
- ActionOption;
- Decision;
- ActionResult;
- AgentPsychState mínimo.

## Etapa 2 — runtime mínimo

- registro de agentes;
- OBSERVE;
- DECIDE;
- REPORT;
- explicación mínima.

Sin aprendizaje complejo ni cultura.

## Etapa 3 — creencias + appraisal + emociones

Hacer funcionar Mara de punta a punta.

## Etapa 4 — utility + histéresis

Validar decisiones divergentes y continuidad de acción.

## Etapa 5 — memoria + relaciones + metas

Dar continuidad temporal al agente.

## Etapa 6 — inferencia declarativa

Añadir hipótesis y evidencia contradictoria.

## Etapa 7 — aprendizaje simple

`ActionType + ContextTags -> OutcomeTags`.

## Etapa 8 — export/import + validación hostil

Habilitar portabilidad real del estado.

## Etapa 9 — batch + rendimiento + paralelización

Sólo después de que el resultado individual esté estabilizado.

## Etapa 10 — fachada C y conectores adicionales

Cuando el core C++ tenga contrato suficientemente estable.

---

# 22. Decisiones que todavía NO se fijan

Aún deben validarse mediante sandbox:

- fórmula exacta de utility;
- normalización de pesos;
- valores numéricos de cuotas;
- tamaño máximo recomendado de memorias/creencias;
- representación física definitiva de proposiciones;
- estrategia exacta de internado de strings/conceptos;
- contenedor concreto para AgentStore;
- formato de serialización;
- threading model final;
- ABI C concreta;
- build system definitivo.

---

# 23. Criterio de éxito de esta arquitectura

La arquitectura es válida si:

1. un juego puede integrar MSPA sin que MSPA incluya clases de ese juego;
2. dos hosts diferentes pueden usar el mismo core y contrato;
3. un agente puede exportarse/importarse sin punteros ni dependencias del host;
4. la misma entrada reproduce la misma decisión bajo las garantías declaradas;
5. un Domain Pack externo no puede ejecutar código arbitrario ni saltarse cuotas;
6. el sandbox puede explicar por qué cuatro agentes distintos reaccionan de forma distinta al mismo escenario;
7. el runtime puede escalar a procesamiento batch sin cambiar el significado de una evaluación individual.

Hasta que estos criterios no se demuestren, esta arquitectura sigue siendo **candidata**, no congelada.
