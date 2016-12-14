<?php 
     if(!is_object($this))
      {
        exit; 
      }   
 
 $ID = ID;
 $COLOR_BLANCO=COLOR_BLANCO;
$COLOR_BORDE_TABLA_FM_INSERT=COLOR_BORDE_TABLA_FM_INSERT;

 
$PATH_IMAGENES = PATH_IMAGENES; $PATH_ICONOS = PATH_ICONOS; $FORMATO_IMAGEN_ICONOS = FORMATO_IMAGEN_ICONOS;
                        
$PATH_HERRAMIENTAS = PATH_HERRAMIENTAS;
                        
$COLOR_BORDE_TABLA = COLOR_CLARO_DOMINANTE_FM_INSERT;
                        
$COLOR_CLARO_DOMINANTE = COLOR_BORDE_TABLA_FM_INSERT;
                        
$GRIS = GRIS;                        
                        
$COLOR_BLANCO = COLOR_BLANCO;



//-------------------------------------------------------------------------------------------------------------

    require_once PATH_PAQUETE."/nomina/novedad/novedadporpersona/fm_NovedadPrivilegios.php";

//-------------------------------------------------------------------------------------------------------------

/*if ($ndn_titulos_obtenidos == NULL)
	$ndn_titulos_obtenidos = 0;

if ($ndn_categoria == NULL)
	$ndn_categoria = 0;

if ($ndn_experiencia_calificada == NULL)
	$ndn_experiencia_calificada = 0;

if ($ndn_produccion_academica == NULL)
	$ndn_produccion_academica = 0;

if ($ndn_direccion_academica == NULL)
	$ndn_direccion_academica = 0;

if ($ndn_asignacion_adicional == NULL)
	$ndn_asignacion_adicional = 0;

if ($ndn_docencia_destacada == NULL)
	$ndn_docencia_destacada = 0;

if ($ndn_diferencia_puntos == NULL)
	$ndn_diferencia_puntos = 0;*/

print "
        <input type=\"HIDDEN\" name=\"ndc_ten_codigo\" value=\"$ndc_ten_codigo\" >

        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Estudios t&iacute;tulos obtenidos: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndc_titulos_obtenidos'  value ='$ndc_titulos_obtenidos' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Labor docente: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndc_labor_docente'  value ='$ndc_labor_docente' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Experiencia profesional: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndc_experiencia_profesional'  value ='$ndc_experiencia_profesional' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Experiencia acad&eacute;mica: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndc_experiencia_academica'  value ='$ndc_experiencia_academica' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Producci&oacute;n intelectual: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndc_produccion_intelectual'  value ='$ndc_produccion_intelectual' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Idiomas: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndc_idiomas'  value ='$ndc_idiomas' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Distinci&oacute;n acad&eacute;mica: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndc_distincion_academica'  value ='$ndc_distincion_academica' size=\"5\">
            </td>
        </tr>

         

       <tr>
            <td class=\"normalNegroB\">&nbsp;</td>
       </tr>

";

?>
