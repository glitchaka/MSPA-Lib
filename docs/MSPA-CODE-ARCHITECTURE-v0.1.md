# MSPA-Lib — Arquitectura de código v0.1

> Arquitectura implementada para el núcleo v1.0.0. Agnóstica al host.

## 1. Lenguaje y build

C++17, CMake 3.20+. Build estático o compartido. Política numérica: `/fp:strict` en MSVC; GCC/Clang sin fast-math y sin contracción FMA. Paquete instalable como `MSPA::mspa`.

## 2. Módulos

```text
base
  ↓
semantics
  ↓
psychology
  ↓
decision
  ↓
runtime
```

Laterales:

```text
validation  -> base + public model
persistence -> base + public model
 diagnostics -> public model + traces
C ABI        -> public runtime API
```

No existen dependencias hacia consumidores concretos.

## 3. Base

IDs fuertes, `PsychTime`, `TimeDomain`, `Result<T>`, errores, versión y RNG determinista. `AgentId` incluye `WorldId`.

## 4. Semantics

`Proposition`, `PredicateSchema`, `ActionSchema`, `ConsiderationSchema`, `RegistrySnapshot/Builder`, inferencia acotada y Domain Packs. Registry activo inmutable; staging/publicación entre batches.

## 5. Psychology

`AgentPsychState` es el ownership unitario por agente. Subsistemas: personality, values, beliefs, appraisal, emotions, mood, goals, memory, relationships, learning, reappraisal y engine coordinador.

## 6. Decision

Utility AI consume `ActionOption`, `DecisionContext`, estado psicológico y registry. Curvas son lineales por tramos. Histéresis usa continuidad, persistencia, progreso, interrupción, contexto y urgencia.

## 7. Runtime

`Runtime` posee config, registry snapshot, staging opcional, `AgentStore`, psychology, utility y diagnostics. APIs: lifecycle, observe/decide/report/advance/react, batch, export/import, staging de packs.

Una instancia no admite mutaciones concurrentes no sincronizadas por el host. Instancias independientes no comparten estado mutable obligatorio.

## 8. Persistence

Transferencia de estado convierte dominios temporales y RNG según política. Serialización binaria versionada valida magic/schema/longitudes/cuotas y permite migración controlada.

## 9. Validation

`HardLimits`, `DefaultLimits`, `RuntimeLimits`. Validación profunda de estado/proposiciones y payload serializado. Ninguna entrada externa se considera confiable.

## 10. Diagnostics

`TraceSink` acotado. Tokens permiten recuperar trazas sin mezclar el estado simulacional con instrumentación.

## 11. ABI C

`mspa/c_api.h` expone `mspa_runtime*` opaco y status codes. Incluye lifecycle, observe, decide, report, advance y export/import binario. El header se valida con compilador C.

## 12. Determinismo

Orden batch `(AgentId,input_sequence)`, RNG por agente, mapas ordenados en rutas deterministas, snapshots inmutables y escala temporal entera/racional.

## 13. Build targets

Biblioteca: `mspa` / `MSPA::mspa`.

Tests: core, semantics, psychology, runtime, persistence, C API, security y C header.

Opcionales: benchmark y fuzz de deserialización.

## 14. Invariantes de ownership

- host posee realidad y ejecución;
- runtime posee estados psicológicos activos;
- cada `AgentPsychState` pertenece a un runtime a la vez;
- snapshots semánticos se comparten sólo como datos inmutables;
- referencias externas son IDs, nunca punteros del host;
- serialización no contiene direcciones de proceso.

## 15. Errores

Las APIs C++ de runtime devuelven `Result<T>` y capturan `bad_alloc`/excepciones inesperadas. La ABI C las traduce a `mspa_status`; ninguna excepción cruza ABI.

## 16. Fuera de alcance

Adaptadores específicos, física, rendering, navegación, mundo y scheduler del host no forman parte de MSPA-Lib.
