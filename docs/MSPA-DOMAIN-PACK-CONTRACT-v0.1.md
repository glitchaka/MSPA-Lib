# MSPA Domain Pack Contract v0.1

## Propósito

Un **Domain Pack** describe semántica psicológicamente relevante de un host sin introducir lógica específica de ese host dentro del núcleo MSPA.

MSPA define el formato, lo valida y lo consume. Los Domain Packs concretos viven fuera del repositorio MSPA.

Este documento no contiene nombres, escenarios, entidades ni reglas de proyectos consumidores.

## 1. Metadatos obligatorios

Todo pack declara conceptualmente:
- `pack_id`
- versión
- namespace
- dependencias
- compatibilidad
- versión mínima de MSPA
- recomendaciones de recursos.

Las recomendaciones nunca elevan los límites impuestos por el runtime o el host.

## 2. Vocabulario de dominio

El pack registra conceptos y predicados que no pertenecen al vocabulario base MSPA.

Todo identificador de dominio utiliza un namespace propio. Siempre que sea posible, puede mapear parte de su significado a categorías o predicados base MSPA.

## 3. Predicate Schemas

Cada predicado interpretable declara:
- roles;
- cardinalidad;
- rol con agencia;
- posibles objetivos;
- categorías de consecuencia;
- categorías de relevancia;
- intencionalidad posible;
- mapeos a vocabulario base;
- `ContextTags` relevantes;
- `OutcomeTags` posibles;
- pistas de appraisal.

El schema no fija una emoción ni una valoración universal.

## 4. Event Templates

Un Event Template describe cómo un hecho del host se traduce a proposiciones y pistas semánticas.

Puede declarar:
- identificador de evento de dominio;
- proposiciones producidas;
- roles relevantes;
- saliencia base;
- información que el host debe completar;
- Predicate Schemas involucrados.

El pack no decide quién percibió realmente el evento.

## 5. Action Schemas

Cada tipo de acción puede declarar:
- `ActionTypeId`;
- roles;
- objetivo(s);
- consideraciones psicológicas relevantes;
- `ContextTags`;
- posibles `OutcomeTags`;
- riesgos/costes semánticos;
- mapeos a categorías base.

## 6. Consideraciones y curvas

El pack puede declarar qué consideraciones usa cada acción y cómo transformar entradas normalizadas.

Las curvas deben ser:
- serializables;
- editables;
- acotadas;
- reproducibles;
- preferiblemente lineales por tramos.

No se permite código arbitrario dentro de curvas.

## 7. Reglas de inferencia

Las reglas deben ser declarativas y usar exclusivamente predicados autorizados.

Cada regla puede declarar:
- condiciones;
- hipótesis resultante;
- certeza base;
- dependencias;
- prioridad/coste lógico;
- categorías de evidencia.

No puede:
- ejecutar código nativo;
- acceder a filesystem o red;
- crear recursión ilimitada;
- exceder presupuestos definidos por MSPA.

## 8. Aprendizaje

Para aprendizaje causal inicial, el pack declara por acción:
- `ContextTags` permitidos;
- `OutcomeTags` permitidos;
- límite de combinaciones;
- condiciones para registrar experiencia;
- pesos iniciales opcionales.

El runtime aprende estadísticas sobre combinaciones autorizadas, no sobre contexto arbitrario del mundo.

## 9. Valores y normas iniciales

Un pack puede ofrecer catálogos iniciales de:
- valores;
- normas personales;
- normas sociales percibidas;
- conceptos de prestigio;
- prácticas conocidas.

Esto no crea una cultura global. Sólo proporciona vocabulario y datos iniciales que el host puede usar al crear agentes.

## 10. Influencia externa

El pack puede registrar clases semánticas de influencia externa. No puede imponer obediencia ni resultados psicológicos predeterminados.

## 11. Portabilidad

Un pack debe declarar qué conceptos son:
- portables;
- locales al mundo;
- resolubles por namespace;
- opacos pero preservables.

Las referencias externas pueden sobrevivir aunque no puedan resolverse localmente.

## 12. Compatibilidad

El pack permite comprobar:
- misma versión;
- versiones compatibles;
- vocabulario compartido;
- dependencias ausentes;
- predicados desconocidos;
- reglas no autorizadas.

Una creencia importada no se elimina sólo porque el runtime receptor no comprenda uno de sus predicados; puede conservarse de forma opaca.

## 13. Seguridad y presupuestos

Todo pack externo se considera entrada no confiable.

Se limitan:
- cantidad de predicados;
- Event Templates;
- Action Schemas;
- reglas de inferencia;
- tamaño de reglas;
- profundidad lógica;
- `ContextTags`;
- `OutcomeTags`;
- curvas;
- strings;
- tamaño total.

El pack nunca incluye código ejecutable arbitrario.

## 14. Core Psychology Pack

MSPA puede incluir semántica psicológica general:
- vocabulario base;
- rasgos base;
- emociones base;
- relaciones;
- categorías generales de appraisal;
- predicados sociales comunes;
- inferencias mínimas genéricas;
- consideraciones generales;
- curvas de referencia.

Ningún contenido del Core Psychology Pack debe depender de un host concreto.

## 15. Invariante de separación

Un Domain Pack concreto no forma parte del diseño normativo de MSPA ni del core. Debe poder desarrollarse, versionarse y distribuirse desde el proyecto consumidor.

Si una integración requiere añadir al core nombres, clases, eventos o reglas propias del host, la frontera está mal definida y debe corregirse.