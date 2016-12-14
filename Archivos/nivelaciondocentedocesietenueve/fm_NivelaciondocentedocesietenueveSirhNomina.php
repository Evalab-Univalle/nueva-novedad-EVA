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
        <input type=\"HIDDEN\" name=\"ndn_ten_codigo\" value=\"$ndn_ten_codigo\" >

        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> T&iacute;tulos obtenidos: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndn_titulos_obtenidos'  value ='$ndn_titulos_obtenidos' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Categor&iacute;a: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndn_categoria'  value ='$ndn_categoria' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Experiencia calificada: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndn_experiencia_calificada'  value ='$ndn_experiencia_calificada' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Producci&oacute;n acad&eacute;mica: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndn_produccion_academica'  value ='$ndn_produccion_academica' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Direcci&oacute;n acad&eacute;mica: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndn_direccion_academica'  value ='$ndn_direccion_academica' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Asignaci&oacute;n adicional: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndn_asignacion_adicional'  value ='$ndn_asignacion_adicional' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Docencia destacada: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndn_docencia_destacada'  value ='$ndn_docencia_destacada' size=\"5\">
            </td>
        </tr>
        <tr>
            <td class=\"normalNegroB\"><label accesskey='n'> Diferencia puntos: </td>
            <td class=\"normalNegro\">
                <input type='TEXT'  class=\"normalNegro\" name='ndn_diferencia_puntos'  value ='$ndn_diferencia_puntos' size=\"5\">
            </td>
        </tr>

         

       <tr>
            <td class=\"normalNegroB\">&nbsp;</td>
       </tr>

";

?>
