# MSPA-Lib — Especificación lógica v2.1

## 1. Propósito

MSPA-Lib es un **Motor de Simulación Psicológica de Agentes** diseñado para ser reutilizable en distintos juegos y simulaciones. Su función no es ejecutar acciones del mundo, sino transformar percepciones y contexto en estados psicológicos, creencias, relaciones, metas y preferencias de conducta explicables.

La librería debe poder acoplarse a proyectos muy distintos sin conocer sus sistemas internos. Un agente puede representar un aldeano, soldado, colono, criatura, ciudadano, personaje de RPG o cualquier otra entidad autónoma.

MSPA no controla movimiento, navegación, combate, animaciones, recursos, inventarios, escenas ni reglas físicas. El proyecto anfitrión conserva toda esa autoridad.

---

## 2. Contrato fundamental

El ciclo conceptual es:

1. El mundo produce un acontecimiento.
2. El proyecto anfitrión determina qué agentes pueden percibirlo y con qué claridad.
3. MSPA filtra atención y relevancia.
4. El agente forma o actualiza creencias.
5. Evalúa el acontecimiento según personalidad, valores, metas, relaciones, normas y memoria.
6. Se generan emociones.
7. El agente puede intentar regularlas.
8. Se actualizan ánimo, relaciones y memoria.
9. El proyecto anfitrión ofrece las acciones posibles.
10. MSPA las valora.
11. Una acción obtiene la preferencia dominante, respetando histéresis y restricciones reales.
12. El proyecto ejecuta la acción.
13. El resultado vuelve a MSPA como nuevo acontecimiento.

Principio central:

> **MSPA procesa significado psicológico; el host controla realidad y ejecución.**

---

## 3. Fronteras del sistema

### MSPA sí conoce

- agentes e identificadores;
- rasgos de personalidad;
- valores y su intensidad;
- metas y compromisos;
- creencias subjetivas;
- emociones;
- ánimo;
- relaciones;
- memorias;
- normas culturales;
- acontecimientos abstractos;
- opciones de acción abstractas;
- resultados de acción;
- tiempo lógico;
- explicaciones causales.

### MSPA no conoce

- sprites o modelos;
- coordenadas gráficas específicas del motor;
- pathfinding;
- animaciones;
- inventarios concretos del juego;
- sistemas de combate;
- navegación;
- escenas;
- físicas;
- interfaces gráficas;
- archivos de guardado del proyecto;
- red;
- audio;
- hilos propios obligatorios;
- conceptos exclusivos de un juego concreto.

Nada equivalente a “isla”, “grano”, “muelle”, “milagro”, “poción” o “arma láser” debe estar codificado dentro del núcleo. Esos conceptos pertenecen al host.

---

## 4. Identificadores opacos

MSPA debe trabajar con referencias abstractas en vez de tipos específicos del juego.

Se consideran necesarios, conceptualmente:

- AgentId
- EntityId
- EventId
- ActionId
- GoalId
- BeliefId
- MemoryId
- ConceptId
- CultureId
- LocationId

MSPA no interpreta el contenido de esos identificadores salvo mediante metadatos registrados por el host.

---

# PARTE I — IDENTIDAD Y MOTIVACIÓN

## 5. Personalidad

La personalidad representa predisposiciones relativamente estables. No debe cambiar por estados momentáneos.

Se fijan seis ejes base:

| Eje | Bajo | Alto | Función principal |
|---|---|---|---|
| Sociabilidad | reservado | sociable | peso de interacción y compañía |
| Estabilidad | reactivo | sereno | intensidad y duración emocional |
| Empatía | egocéntrico | empático | sensibilidad al estado de otros |
| Persistencia | flexible | obstinado | abandono o continuidad de metas, resistencia a cambiar conclusiones |
| Curiosidad | tradicional | curioso | interés por novedad e información |
| Audacia | prudente | temerario | peso del riesgo y disposición al peligro |

### Regla de diseño

La personalidad **no añade bonificaciones directas a acciones**.

Ejemplos:

- Audacia no significa “cazar +20”. Significa que el riesgo pesa menos.
- Empatía no significa “ayudar +20”. Significa que el bienestar ajeno entra con mayor fuerza en la evaluación.
- Sociabilidad no significa “hablar +20”. Significa que la interacción social ofrece más utilidad subjetiva.
- Persistencia no significa “trabajar +20”. Significa que abandonar una meta o una creencia consolidada cuesta más.

### Cambio durante la vida

Puede existir cambio de personalidad, pero:

- sólo por experiencias extremas o desarrollo prolongado;
- en cantidades pequeñas;
- con límites temporales;
- sin volver irreconocible al agente.

---

## 6. Valores

Los valores indican **qué considera importante** el agente. El catálogo no debe estar codificado rígidamente en el núcleo.

MSPA necesita únicamente:

- identificador del valor;
- intensidad;
- posible pertenencia cultural;
- posible conflicto con otros valores.

Un proyecto puede registrar valores como:

- familia;
- comunidad;
- prosperidad;
- conocimiento;
- fe;
- autonomía;
- prestigio.

Otro proyecto podría usar honor, imperio, naturaleza, disciplina o magia sin modificar el núcleo.

### Diferencia entre personalidad y valor

- Personalidad: cómo tiendo a reaccionar.
- Valor: qué considero importante.

Ejemplo:

Un agente puede valorar mucho la familia y ser poco empático. Aun así puede priorizarla por deber, tradición o identidad.

---

## 7. Necesidades

Las necesidades físicas o sistémicas pertenecen principalmente al host.

Ejemplos:

- hambre;
- sed;
- sueño;
- temperatura;
- seguridad;
- dolor.

MSPA recibe su relevancia y puede usarlas como motivadores, pero no debe imponer un catálogo obligatorio.

Principio:

> Una necesidad no es una emoción.

El hambre puede generar ansiedad, miedo o irritación según intensidad, contexto y creencias, pero sigue siendo una necesidad.

---

## 8. Metas activas

Las metas convierten valores y necesidades en objetivos concretos.

Cada meta debe tener conceptualmente:

- origen;
- objeto o condición objetivo;
- prioridad;
- estado;
- condición de cierre;
- contexto temporal;
- posibilidad de suspensión.

### Orígenes posibles

- necesidad;
- valor;
- compromiso;
- evento;
- creencia;
- relación;
- regulación emocional.

### Estados

- activa;
- suspendida;
- bloqueada;
- cumplida;
- abandonada.

### Principio de legibilidad

Un agente puede tener muchas preocupaciones, pero sólo unas pocas metas dominantes a la vez. El diseño inicial debe favorecer aproximadamente 1–3 metas activas principales.

---

## 9. Compromisos

Los compromisos no forman un sistema independiente. Son **un origen de meta**.

Ejemplos:

- promesa;
- tarea aceptada;
- juramento;
- obligación familiar;
- responsabilidad de cuidado;
- deber profesional.

Esto permite que un agente actúe contra una emoción momentánea porque una obligación mantiene prioridad.

---

# PARTE II — PERCEPCIÓN Y CONOCIMIENTO SUBJETIVO

## 10. Verdad del mundo y creencia del agente

MSPA nunca debe asumir que el agente conoce la verdad global.

El host conoce el hecho real.

El agente conoce una percepción parcial, una inferencia o una versión transmitida.

Ejemplo:

Verdad:

> Torvando intentó salvar a Mara y no llegó a tiempo.

Percepción de Lencio:

> Torvando estaba allí y Mara murió.

Creencia posible:

> Torvando pudo salvarla y decidió no hacerlo.

Toda evaluación emocional posterior puede basarse en esa creencia, aunque sea falsa.

---

## 11. Filtro de percepción y atención

No todo evento llega al sistema psicológico.

Antes de evaluar debe pasar tres filtros:

### 11.1 Acceso perceptivo

El host determina si el agente:

- vio;
- escuchó;
- fue informado;
- observó indirectamente;
- no tuvo acceso.

### 11.2 Relevancia

El evento toca:

- una meta activa;
- un valor importante;
- una relación significativa;
- una necesidad;
- una amenaza;
- una norma relevante.

### 11.3 Saliencia

Eventos extraordinarios pueden imponerse aunque no estuvieran ligados previamente a una meta.

Ejemplos:

- explosión cercana;
- muerte;
- ataque;
- desastre;
- celebración masiva.

Si un evento no pasa el filtro, no necesita generar emoción, creencia ni memoria.

---

## 12. Creencias

Una creencia representa una afirmación que el agente considera más o menos probable.

Debe contener conceptualmente:

- proposición;
- certeza;
- fuente;
- origen;
- carga emocional asociada;
- última activación;
- posibles entidades implicadas.

### Origen de una creencia

- observación directa;
- inferencia;
- información transmitida;
- recuerdo;
- deducción a partir de varias creencias.

No todas las fuentes pesan igual.

---

## 13. Actualización de creencias

La evidencia nueva no sustituye automáticamente una creencia anterior.

Su peso depende de:

- certeza previa;
- carga emocional de la creencia;
- confianza en la fuente;
- respeto hacia la fuente;
- familiaridad;
- claridad perceptiva;
- Persistencia;
- Curiosidad;
- emoción activa;
- compatibilidad con otras creencias.

Esto crea sesgo de confirmación.

### Principio

> La misma evidencia puede producir conclusiones distintas en agentes distintos.

---

## 14. Sesgo de confirmación

El sesgo de confirmación forma parte del núcleo.

Una evidencia contradictoria pesa menos cuando:

- la creencia anterior es fuerte;
- la creencia tiene gran carga emocional;
- la fuente genera poca confianza;
- el agente tiene Persistencia alta;
- existe ira activa hacia el acusado;
- aceptar la evidencia amenaza un valor o una meta.

Pesa más cuando:

- la fuente es confiable;
- existe alto respeto y familiaridad;
- Curiosidad es alta;
- la evidencia es clara;
- varias fuentes independientes coinciden.

---

## 15. Deriva y mantenimiento de certeza

Una creencia no debe aumentar su certeza simplemente porque pasa el tiempo.

Puede fortalecerse mediante:

- repetición;
- reactivación frecuente;
- nueva evidencia compatible;
- múltiples fuentes;
- uso reiterado para interpretar otros eventos.

Puede debilitarse mediante:

- falta prolongada de activación;
- evidencia contradictoria;
- pérdida de confianza en la fuente;
- nuevas interpretaciones;
- reapreciación.

---

## 16. Transmisión de creencias

No se requiere inicialmente un “sistema de rumores” independiente.

Una conversación u otro canal social puede transmitir una afirmación.

En cada transmisión pueden variar:

- certeza;
- causalidad atribuida;
- intención percibida;
- omisión de contexto;
- énfasis emocional.

Así los rumores pueden emerger del mismo mecanismo de creencias.

---

# PARTE III — EVALUACIÓN EMOCIONAL

## 17. Evaluación cognitiva

Un evento no produce una emoción de forma directa. El agente evalúa **lo que cree que ocurrió**.

Los factores de evaluación son:

### Relevancia

¿Me importa?

### Consecuencia

¿Me beneficia, me perjudica o es neutral?

### Agencia

¿Quién creo que lo causó?

### Intención percibida

¿Fue deliberado, accidental o incierto?

### Certeza

¿Qué tan seguro estoy de mi interpretación?

### Control

¿Puedo hacer algo al respecto?

### Novedad

¿Era esperado?

### Norma

¿Lo considero aceptable según mis normas y cultura?

### Relación

¿Qué significan para mí las entidades implicadas?

No todos los factores deben pesar igual en todos los eventos.

---

## 18. Seis emociones básicas

MSPA v2.1 congela seis emociones fundamentales:

| Emoción | Función | Tendencia típica |
|---|---|---|
| Alegría | beneficio, logro, experiencia positiva | mantener, compartir, acercarse |
| Tristeza | pérdida o deterioro significativo | retirarse, buscar apoyo, procesar |
| Miedo | peligro concreto | huir, protegerse, pedir ayuda |
| Ira | daño atribuido a un agente responsable | confrontar, corregir, castigar |
| Ansiedad | amenaza futura o incertidumbre | vigilar, prepararse, buscar información |
| Aversión | rechazo físico, social o moral | evitar, excluir, alejarse |

### Diferencia entre Ira y Aversión

Ira:

> “Esto me dañó y quiero responder.”

Aversión:

> “No quiero esto cerca de mí.”

Una persona puede sentir ambas, una sola o ninguna.

---

## 19. Emociones como instancias

Las emociones no son simples barras globales.

Cada instancia debe poder mantener:

- tipo;
- intensidad;
- objetivo;
- causa percibida;
- origen;
- duración estimada;
- memoria o creencia asociada.

Ejemplo conceptual:

> Ira alta hacia Torvando por la creencia de que abandonó a Mara.

Esto permite sentir ira hacia una persona y simultáneamente tristeza por otra.

---

## 20. Emociones derivadas

No necesitan nuevas barras independientes. Son interpretaciones de combinaciones existentes.

Ejemplos:

- Orgullo = alegría + logro propio + prestigio.
- Gratitud = beneficio + atribución positiva a otra persona.
- Culpa = tristeza + responsabilidad propia + norma/empatía.
- Vergüenza = evaluación negativa propia + exposición social.
- Celos = ansiedad + amenaza a vínculo importante.
- Duelo = tristeza intensa + pérdida de relación significativa.
- Resentimiento = ira persistente + memoria + atribución de culpa.
- Alivio = disminución fuerte de miedo o ansiedad.
- Esperanza = expectativa positiva sobre una meta incierta.

---

## 21. Regulación emocional

Un agente no sólo siente; también intenta gestionar lo que siente.

Posibles estrategias:

- evitar;
- confrontar;
- buscar apoyo;
- distraerse;
- rezar;
- rumiar;
- aislarse;
- reconciliarse;
- buscar información;
- prepararse;
- expresar;
- contenerse.

La personalidad, las normas y las relaciones influyen en qué estrategia resulta más atractiva.

---

## 22. Emoción sentida y emoción expresada

El estado interno y el comportamiento visible no son equivalentes.

Un agente puede:

- sentir miedo y aparentar calma;
- sentir ira y mantener cortesía;
- sentir tristeza y trabajar normalmente;
- sentir aversión sin expresarla.

Los demás agentes sólo deberían reaccionar inicialmente a lo observable, salvo que tengan otra fuente de información.

Esto permite malentendidos sociales.

---

# PARTE IV — ÁNIMO Y MEMORIA

## 23. Ánimo

El ánimo resume el clima afectivo reciente y cambia más lentamente que una emoción.

Se representa mediante dos dimensiones:

### Valencia

negativa ↔ positiva

### Activación

calmada ↔ alterada

Combinaciones típicas:

| Valencia | Activación | Lectura posible |
|---|---|---|
| positiva | alta | entusiasmado |
| positiva | baja | satisfecho |
| negativa | alta | tenso / irritable |
| negativa | baja | abatido |

El ánimo modifica umbrales y pesos, pero no debe controlar decisiones por sí solo.

### Línea basal

Cada agente tiende lentamente hacia un estado basal determinado en parte por personalidad y experiencias prolongadas.

Esto evita estados negativos eternos o neutralización instantánea.

---

## 24. Memoria emocional

No se almacenan todos los eventos.

Una memoria relevante debe contener:

- acontecimiento recordado;
- personas implicadas;
- interpretación del agente;
- emoción original;
- importancia;
- fuente;
- certeza;
- fecha;
- última reactivación;
- posibles vínculos con otras memorias.

Ejemplo:

> “Creo que Torvando abandonó a Mara durante el incendio.”

La memoria incluye el juicio, no sólo el hecho.

---

## 25. Niveles de memoria

### Trivial

Desaparece rápidamente.

### Ordinaria

Puede durar semanas, meses o años según importancia y reactivación.

### Vital

Puede permanecer toda la vida, aunque su carga emocional cambie.

Una memoria puede convertirse en vital por combinación de:

- intensidad extrema;
- relevancia identitaria;
- repetición;
- reactivación frecuente;
- consecuencias prolongadas.

---

## 26. Reactivación contextual

Una memoria puede reaparecer cuando existen pistas relacionadas:

- una persona;
- un lugar;
- un tipo de evento;
- una conversación;
- una fecha;
- una situación similar.

La reactivación puede influir temporalmente en emoción, creencias y decisiones.

---

## 27. Reapreciación

Reapreciar no es lo mismo que cambiar una creencia.

### Actualización de creencia

> “Ahora creo que Torvando sí intentó salvarla.”

### Reapreciación

> “Sigo creyendo que falló, pero ya no siento la misma ira.”

Cuando una memoria se reactiva, su carga emocional puede recalcularse según:

- ánimo actual;
- relación actual;
- nuevas memorias;
- nuevas creencias;
- distancia temporal;
- regulación previa.

Esto permite duelo, perdón parcial y cambio emocional sin exigir nueva evidencia externa.

---

# PARTE V — RELACIONES Y CULTURA

## 28. Relaciones direccionales

Cada relación A → B es independiente de B → A.

MSPA usa cuatro dimensiones:

### Afecto

Cuánto aprecio a esa persona.

### Confianza

Cuánto espero que actúe de forma favorable o predecible conmigo.

### Respeto

Cuánto valoro su capacidad, posición o criterio.

### Familiaridad

Cuánto realmente conozco a esa persona.

Ejemplos válidos:

- mucho afecto, poca confianza;
- mucho respeto, poco afecto;
- confianza moderada, poca familiaridad;
- familiaridad alta y aversión alta.

---

## 29. Parentesco

El parentesco no implica amor automático permanente.

Puede establecer predisposiciones iniciales, responsabilidades o metas, pero las relaciones deben seguir modificándose por experiencia.

---

## 30. Normas culturales

Las normas representan expectativas compartidas.

Ejemplos posibles:

- compartir comida durante una crisis;
- proteger menores;
- respetar ancianos;
- condenar agresión pública;
- desconfiar de extranjeros;
- respetar lugares sagrados.

Las normas afectan:

- evaluación moral;
- culpa;
- vergüenza;
- respeto;
- expresión emocional;
- coste subjetivo de una acción.

El catálogo debe ser definido por el host.

---

## 31. Restricciones reales y tabúes

Debe distinguirse entre dos cosas.

### Restricciones reales

Eliminan una acción del espacio de posibilidades.

Ejemplos:

- un bebé no puede talar un árbol;
- no se puede beber agua inexistente;
- no se puede conversar con alguien inaccesible.

Estas restricciones las determina principalmente el host.

### Normas y tabúes

No eliminan automáticamente una acción. Introducen un coste psicológico y social muy alto.

Esto permite que, bajo condiciones extremas, algunos agentes rompan normas.

Si un proyecto desea prohibir absolutamente cierta conducta, eso debe considerarse una regla del mundo o del diseño, no una propiedad psicológica.

---

# PARTE VI — DECISIÓN

## 32. Generación de acciones

MSPA no inventa movimientos concretos del juego.

El host proporciona las acciones actualmente posibles.

Ejemplos abstractos:

- comer;
- hablar con alguien;
- continuar tarea;
- huir;
- investigar;
- descansar;
- acercarse;
- evitar;
- ayudar;
- confrontar.

Cada proyecto puede definir sus propias acciones.

---

## 33. Utilidad de acción

La decisión se basa en una puntuación conceptual aditiva y normalizada.

Factores posibles:

- necesidad;
- meta activa;
- valores;
- emoción;
- relación;
- memoria relevante;
- norma;
- contexto;
- coste;
- riesgo.

La personalidad modifica pesos y sensibilidad.

No se fija todavía una fórmula numérica definitiva.

---

## 34. Histéresis

Un agente no debe cambiar de acción constantemente por fluctuaciones menores.

Una acción iniciada recibe compromiso temporal.

Se abandona cuando:

- finaliza;
- deja de ser posible;
- aparece una necesidad urgente;
- ocurre un peligro;
- una alternativa supera claramente la utilidad de continuar.

La histéresis es parte obligatoria del sistema de decisión.

---

## 35. Ruido bajo

Puede existir variación pseudoaleatoria cuando varias acciones tienen utilidad similar.

El ruido debe ser pequeño.

Objetivo:

> impredecibilidad razonable, no incoherencia.

---

## 36. Resultado de acción

El host devuelve el resultado de ejecutar la acción.

Ejemplos:

- éxito;
- fracaso;
- interrupción;
- rechazo social;
- imposibilidad sobrevenida.

El resultado puede generar nuevos eventos y emociones.

---

# PARTE VII — TIEMPO Y DECAIMIENTO

## 37. Tiempo lógico desacoplado

MSPA no debe usar tiempo real del sistema como fundamento psicológico.

El host avanza el tiempo lógico.

Esto permite:

- pausa;
- velocidades múltiples;
- juegos por turnos;
- días acelerados;
- simulaciones sin interfaz.

---

## 38. Escalas temporales

| Capa | Escala conceptual |
|---|---|
| Emoción | segundos a días |
| Regulación emocional | minutos a días |
| Ánimo | días |
| Meta inmediata | minutos a días |
| Meta personal | días a años |
| Relación | días a años |
| Creencia | variable |
| Memoria ordinaria | semanas a años |
| Memoria vital | potencialmente toda la vida |
| Personalidad | años |

Las cifras concretas deben ser configurables.

---

# PARTE VIII — REUTILIZACIÓN COMO LIBRERÍA

## 39. Eventos abstractos

El host describe acontecimientos psicológicamente relevantes mediante datos abstractos.

Un evento puede contener:

- tipo;
- actor;
- objetivo;
- entidad afectada;
- magnitud;
- intencionalidad conocida por el host;
- contexto;
- ubicación abstracta;
- etiquetas semánticas.

MSPA no debe depender de nombres concretos de eventos.

---

## 40. Plantillas de evento

Para evitar recalcular toda la semántica desde cero, un proyecto puede registrar arquetipos de eventos.

Ejemplos conceptuales:

- insulto;
- ayuda;
- muerte;
- ataque;
- pérdida;
- regalo;
- descubrimiento.

Una plantilla puede indicar qué aspectos suelen ser relevantes, pero cada agente conserva su evaluación subjetiva.

---

## 41. Percepción como contrato del host

MSPA no debe decidir mediante geometría propia si alguien ve o escucha algo.

El host proporciona:

- quién percibió;
- claridad;
- modalidad;
- posible ruido o distorsión.

Esto permite usar MSPA en 2D, 3D, simuladores por texto o servidores sin gráficos.

---

## 42. Conversación semántica

MSPA no genera diálogo literario.

Debe poder transferir una afirmación entre agentes.

El host decide cómo se representa:

- texto;
- voz;
- animación;
- evento invisible;
- interfaz.

La psicología procesa únicamente la transmisión de información.

---

## 43. Configuración

Cada proyecto debe poder proporcionar un perfil de configuración para ajustar:

- velocidad de decaimiento emocional;
- límites de memoria;
- sensibilidad de relaciones;
- número de metas dominantes;
- fuerza de normas;
- intensidad del sesgo de confirmación;
- ruido decisional;
- línea basal de ánimo;
- frecuencia de actualización;
- módulos habilitados.

---

## 44. Módulos opcionales

La arquitectura debe permitir activar o desactivar componentes.

Ejemplos:

- Personalidad
- Valores
- Metas
- Creencias
- Memoria
- Relaciones
- Regulación emocional
- Expresión emocional

Esto permite diferentes niveles de complejidad.

---

## 45. Presets conceptuales

Se prevén perfiles de uso:

### Minimal

Personalidad + emociones + decisión.

### Social

Añade relaciones y memoria.

### Narrative

Añade creencias, metas y reapreciación.

### Full Simulation

Activa el sistema completo.

Los nombres son provisionales; sirven para definir el objetivo de modularidad.

---

# PARTE IX — DETERMINISMO, PERSISTENCIA Y DIAGNÓSTICO

## 46. Determinismo

Dado el mismo:

- estado inicial;
- orden de eventos;
- avance temporal;
- semilla pseudoaleatoria;

MSPA debe producir el mismo resultado.

Esto es necesario para:

- depuración;
- pruebas;
- guardados;
- multiplayer;
- simulaciones por lotes;
- reproducción de errores.

---

## 47. Persistencia

MSPA debe poder exponer todo su estado psicológico en una representación independiente del sistema de archivos.

El host decide si guarda en:

- JSON;
- binario;
- base de datos;
- formato propio;
- red.

La librería no debe imponer almacenamiento físico.

---

## 48. Versionado

El estado psicológico debe incluir una versión lógica para permitir migraciones futuras.

Esto es obligatorio antes de que existan partidas persistentes a largo plazo.

---

## 49. Explicabilidad

La explicación causal forma parte del producto.

Debe ser posible preguntar conceptualmente:

> ¿Por qué este agente eligió esta acción?

La respuesta debería poder enumerar factores relevantes:

- meta;
- emoción;
- relación;
- memoria;
- valor;
- riesgo;
- norma;
- coste.

Esto sirve tanto al desarrollador como al jugador.

---

## 50. Traza psicológica

Debe existir un modo diagnóstico que permita seguir:

1. qué percibió;
2. qué atendió;
3. qué creyó;
4. cómo evaluó;
5. qué emoción apareció;
6. qué memoria se activó;
7. qué metas se modificaron;
8. qué acciones consideró;
9. qué utilidad recibió cada una;
10. qué acción fue preferida.

Sin esta traza, ajustar un sistema emergente complejo sería extremadamente difícil.

---

# PARTE X — FASES DE IMPLEMENTACIÓN FUTURA

## 51. Fase A — comportamiento individual

Objetivo: comprobar que agentes distintos reaccionan de forma distinta ante el mismo contexto.

Incluye:

- personalidad;
- valores;
- emociones;
- ánimo;
- relaciones;
- utilidad;
- histéresis.

---

## 52. Fase B — continuidad psicológica

Objetivo: comprobar que el pasado modifica el futuro.

Incluye:

- metas;
- memoria;
- creencias;
- reapreciación;
- sesgo de confirmación.

---

## 53. Fase C — sociedad

Objetivo: comprobar que información y estados se propagan socialmente sin scripts específicos.

Incluye:

- transmisión de creencias;
- expresión emocional;
- normas;
- reputación futura;
- rumores emergentes.

---

# PARTE XI — ELEMENTOS POSPUESTOS

## 54. Fuera del núcleo inicial

No se implementarán inicialmente:

- séptimo eje de personalidad;
- ideologías complejas;
- enfermedades mentales;
- personalidad política;
- árbol de rasgos;
- deuda social como estadística permanente;
- ánimo colectivo;
- contagio emocional;
- reputación pública compleja;
- identidad narrativa;
- trauma como módulo independiente;
- sistema dedicado de rumores;
- generación de diálogo literario.

Estas extensiones deben poder añadirse sin rediseñar el núcleo.

---

# PARTE XII — PRUEBAS DE DISEÑO

## 55. Escenario base: incendio de Mara

Verdad del mundo:

> Torvando intentó salvar a Mara y no llegó a tiempo.

Lencio observa sólo parte del acontecimiento y cree:

> Torvando pudo salvarla y no quiso.

Una fuente posterior aporta evidencia favorable a Torvando.

Se deben probar al menos cuatro perfiles:

### Perfil 1

Empático + estable + flexible + curioso.

Resultado esperado:

- reconsidera con relativa facilidad;
- aparece culpa;
- puede generar meta de reconciliación.

### Perfil 2

Reactivo + persistente + poca empatía.

Resultado esperado:

- rechaza evidencia contradictoria;
- mantiene ira;
- refuerza la interpretación previa.

### Perfil 3

Prudente + curioso + empatía media.

Resultado esperado:

- acepta parcialmente;
- conserva desconfianza residual;
- reduce hostilidad sin reconciliación inmediata.

### Perfil 4

Muy empático + reactivo + persistente.

Resultado esperado:

- el fuerte afecto por Mara amplifica el dolor;
- la Persistencia fija la atribución inicial;
- puede ser el agente que más tarda en perdonar pese a su empatía.

Criterio de éxito:

> Los cuatro desenlaces deben emerger de los mecanismos generales, sin reglas especiales para el escenario.

---

## 56. Otros escenarios de validación

Se deberán diseñar escenarios reproducibles para:

- llegada de extranjeros;
- nacimiento;
- pérdida de recursos;
- traición;
- ayuda inesperada;
- escasez;
- accidente;
- reconciliación;
- competencia por una meta;
- falsa acusación;
- rumor repetido;
- ruptura de una norma cultural.

Las pruebas deben medir diversidad conductual, estabilidad, duración de rencores, reconciliación y ausencia de oscilación excesiva.

---

# PARTE XIII — PRINCIPIOS DE CONGELACIÓN

## 57. Núcleo v2.1

Se considera arquitectura congelable:

1. 6 ejes de personalidad.
2. Valores configurables.
3. Necesidades aportadas por el host.
4. Metas activas con compromisos como origen.
5. Percepción y filtro de atención.
6. Creencias con fuente, certeza y sesgo de confirmación.
7. Evaluación cognitiva.
8. 6 emociones básicas + derivadas.
9. Regulación emocional.
10. Emoción sentida y expresada.
11. Ánimo mediante valencia + activación.
12. Relaciones: afecto, confianza, respeto y familiaridad.
13. Memoria con interpretación, reactivación y reapreciación.
14. Normas culturales.
15. Restricciones reales separadas de tabúes.
16. Decisión por utilidad.
17. Histéresis.
18. Ruido bajo y determinista.
19. Escalas temporales diferenciadas.
20. Explicabilidad y traza diagnóstica.

---

## 58. Criterio conceptual final

MSPA está funcionando cuando un rasgo deja de ser una etiqueta moral y se convierte en una predisposición contextual.

Ejemplos:

- Empatía no equivale a bondad.
- Persistencia no equivale a productividad.
- Audacia no equivale a heroísmo.
- Sociabilidad no equivale a amistad.
- Estabilidad no equivale a indiferencia.
- Curiosidad no equivale a inteligencia.

Un rasgo debe poder producir consecuencias positivas o negativas según contexto, valores, relaciones, metas y creencias.

El objetivo final es que historias sociales complejas emerjan de mecanismos generales, no de scripts especiales.

---

## 59. Estado del proyecto

Esta especificación describe **sólo el diseño lógico**.

No autoriza todavía:

- elección definitiva de lenguaje;
- estructura física de código;
- ABI;
- API concreta;
- formatos de serialización;
- bindings;
- CMake;
- dependencias;
- implementación numérica.

Esos puntos se definirán después de validar esta arquitectura.
