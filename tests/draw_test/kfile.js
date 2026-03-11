let project = new Project('Draw Test');

await project.addProject(findKore());

project.addFile('sources/**');
project.addFile('deployment/**');
project.setDebugDir('deployment');

project.addKongDir('shaders');

project.flatten();

resolve(project);
